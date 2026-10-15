const http = require('http');
const fs = require('fs');
const path = require('path');
const crypto = require('crypto');

const PORT = 8080;
const HTML_FILE = path.join(__dirname, 'index.html');

let startTime = Date.now();
let isArmed = false;
let isPerching = false;
let currentThr = 0;
let currentYaw = 0;
let currentPit = 0;
let currentRol = 0;

const server = http.createServer((req, res) => {
  if (req.url === '/' || req.url === '/index.html') {
    res.writeHead(200, { 'Content-Type': 'text/html; charset=utf-8' });
    fs.createReadStream(HTML_FILE).pipe(res);
  } else {
    res.writeHead(404);
    res.end('Not Found');
  }
});

function createWebSocketFrame(payload, isBinary = false) {
  const isBuffer = Buffer.isBuffer(payload);
  const data = isBuffer ? payload : Buffer.from(payload, 'utf8');
  const length = data.length;
  let header;

  const firstByte = 0x80 | (isBinary ? 0x02 : 0x01);

  if (length <= 125) {
    header = Buffer.from([firstByte, length]);
  } else if (length <= 65535) {
    header = Buffer.alloc(4);
    header[0] = firstByte;
    header[1] = 126;
    header.writeUInt16BE(length, 2);
  } else {
    header = Buffer.alloc(10);
    header[0] = firstByte;
    header[1] = 127;
    header.writeBigUInt64BE(BigInt(length), 2);
  }
  return Buffer.concat([header, data]);
}

server.on('upgrade', (req, socket, head) => {
  if (req.url !== '/ws') {
    socket.destroy();
    return;
  }

  const key = req.headers['sec-websocket-key'];
  if (!key) {
    socket.destroy();
    return;
  }

  const GUID = '258EAFA5-E914-47DA-95CA-C5AB0DC85B11';
  const acceptKey = crypto.createHash('sha1').update(key + GUID).digest('base64');

  const responseHeaders = [
    'HTTP/1.1 101 Switching Protocols',
    'Upgrade: websocket',
    'Connection: Upgrade',
    `Sec-WebSocket-Accept: ${acceptKey}`
  ];

  socket.write(responseHeaders.join('\r\n') + '\r\n\r\n');
  console.log('[SIMULATOR] GCS Client connected to /ws');

  let lastPacketTime = Date.now();

  let telemInterval = setInterval(() => {
    if (socket.destroyed) {
      clearInterval(telemInterval);
      return;
    }
    const now = Date.now();
    const linkLost = isArmed && (now - lastPacketTime > 600);
    let modeId = 0;
    if (linkLost) {
      modeId = 3;
    } else if (isPerching) {
      modeId = 2;
    } else if (isArmed) {
      modeId = 1;
    } else {
      modeId = 0;
    }

    const tpkt = Buffer.alloc(12);
    tpkt[0] = 0xBB;
    tpkt[1] = (isArmed && !linkLost ? 1 : 0) | (modeId << 1);
    tpkt[2] = linkLost ? 15 : (isPerching ? 35 : 0);
    tpkt[3] = 90;
    tpkt[4] = 90;
    tpkt[5] = 42;
    tpkt[6] = 160;
    const upSec = Math.floor((Date.now() - startTime) / 1000);
    tpkt.writeUInt16LE(upSec, 7);
    tpkt[9] = Math.floor(upSec / 3600) % 24;
    tpkt[10] = Math.floor(upSec / 60) % 60;
    tpkt[11] = upSec % 60;

    socket.write(createWebSocketFrame(tpkt, true));
  }, 200);

  let buffer = Buffer.alloc(0);

  socket.on('data', (chunk) => {
    buffer = Buffer.concat([buffer, chunk]);

    while (buffer.length >= 2) {
      const firstByte = buffer[0];
      const secondByte = buffer[1];
      const opcode = firstByte & 0x0F;
      const isMasked = (secondByte & 0x80) !== 0;
      let payloadLength = secondByte & 0x7F;
      let offset = 2;

      if (payloadLength === 126) {
        if (buffer.length < 4) break;
        payloadLength = buffer.readUInt16BE(2);
        offset = 4;
      } else if (payloadLength === 127) {
        if (buffer.length < 10) break;
        payloadLength = Number(buffer.readBigUInt64BE(2));
        offset = 10;
      }

      const maskLength = isMasked ? 4 : 0;
      const totalLength = offset + maskLength + payloadLength;
      if (buffer.length < totalLength) break;

      let payload = buffer.slice(offset + maskLength, totalLength);
      if (isMasked) {
        const maskKey = buffer.slice(offset, offset + 4);
        for (let i = 0; i < payload.length; i++) {
          payload[i] ^= maskKey[i % 4];
        }
      }

      buffer = buffer.slice(totalLength);

      if (opcode === 0x08) {
        socket.end();
        break;
      } else if (opcode === 0x01) {
        const text = payload.toString('utf8');
        if (text === 'PING') {
          socket.write(createWebSocketFrame('PONG', false));
        }
      } else if (opcode === 0x02) {
        if (payload.length >= 6 && payload[0] === 0xAA) {
          lastPacketTime = Date.now();
          const flags = payload[1];
          isArmed = (flags & 0x01) !== 0;
          const isStop = (flags & 0x02) !== 0;
          isPerching = (flags & 0x04) !== 0;

          currentThr = payload.readInt8(2);
          currentYaw = payload.readInt8(3);
          currentPit = payload.readInt8(4);
          currentRol = payload.readInt8(5);

          if (isStop || !isArmed || currentThr > 0) {
            isPerching = false;
          }
        }
      }
    }
  });

  socket.on('close', () => {
    clearInterval(telemInterval);
    console.log('[SIMULATOR] GCS Client disconnected');
  });

  socket.on('error', () => {
    clearInterval(telemInterval);
  });
});

server.listen(PORT, () => {
  console.log(`[SIMULATOR] Phelan Web Deck Simulator running at: http://localhost:${PORT}`);
});
