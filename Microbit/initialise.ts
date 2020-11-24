
namespace TargetArchitecture {

    let initialized = false
    let serialNumber: string

    /**
     * Add into the start function to initialise the board.
     * @param value the serial number of the Rainbox Sparkle Unicorn board
     */
    //% blockId=TargetArchitecture_initwithserialnumber
    //% block="Start Rainbow Sparkle Unicorn $value"
    //% weight=65
    export function init(value: string): void {

        serial.redirect(
            SerialPin.P0,
            SerialPin.P1,
            BaudRate.BaudRate115200
        )
        serialNumber = value

        basic.showIcon(IconNames.Happy)

        initialized = true
    }


    serial.onDataReceived(serial.delimiters(Delimiters.NewLine), function () {
        control.raiseEvent(
            MICROBIT_RAINBOW_SPARKLE_UNICORN_TOUCH_SENSOR_TOUCHED_ID,
            2
        );
    })

    basic.forever(function () {
        // microIoT_InquireStatus();
    })


    //% blockId="TargetArchitecture_createMessage"
    //% block="create message $topic $value"      
    //% weight=5
    export function createMessage(topic: string, value: string): string {
        return "{" + serialNumber + "/" + topic + ":" + value + "}\r\n"
    }   

    //% blockId="TargetArchitecture_sendMessage"
    //% block="send Message $topic $value"      
    //% weight=5
    export function sendMessage(topic: string, value: string): void {
        serial.writeString(createMessage(topic, value))
    }


}