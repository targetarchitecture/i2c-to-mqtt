
function sendMessage(str: string) {
    asr_txt = str
    num = asr_txt.length
    let buf2 = pins.createBuffer(num + 4);
    let crcbuf = pins.createBuffer(num);
    buf2[0] = 2
    buf2[1] = num + 4
    for (let j = 0; j <= num - 1; j++) {
        buf2[j + 2] = asr_txt.charCodeAt(j)
        crcbuf[j] = asr_txt.charCodeAt(j)
    }
    buf2[num + 2] = calcCRC8(crcbuf, num)
    buf2[num + 3] = 4
    pins.i2cWriteBuffer(I2C_ADDR, buf2);
    let i2cBuffer = pins.i2cReadBuffer(I2C_ADDR, 32, false);
    for (let k = 0; k <= 32; k++) {
        // find packet start
        if (i2cBuffer[k] == 2) {
            startIndex = k
        }
    }
    readMsgV2(i2cBuffer, startIndex);
}
// sendMessage("A3,World")
input.onButtonPressed(Button.A, function () {
    led.toggle(2, 2)
    sendMessage("A3,Hello World")
})

let a4 = 0
let a3 = 0
let startIndex = 0
let a1 = 0
let asr_txt = ""
let num = 0
let I2C_ADDR = 0
let a2 = 0
function readMsgV2(rxBuf: Buffer, startIndex: number) {
    a1 = rxBuf[startIndex];
    let contentLength = rxBuf[startIndex + 1] - 4;

    for (let l = 0; l < contentLength; l++) {
        a2 = rxBuf[l + 2];
        serial.writeString(String.fromCharCode(a2))
    }
    serial.writeLine("")
}
// basic.forever(function () {
serial.writeLine("HO HO HO")
let DELAY = 150
// let i2cDevice = 99
I2C_ADDR = 4
function calcCRC8(data: Buffer, length: number): number {
    let crc = 0;
    let extract;
    let sum;

    for (let n = 0; n < length; n++) {
        extract = data[n];

        for (let o = 8; o; o--) {
            sum = (crc ^ extract) & 0x01;
            crc >>= 1;
            if (sum) {
                crc ^= 0x8C;
            }
            extract >>= 1;
        }
    }

    return crc;
}
