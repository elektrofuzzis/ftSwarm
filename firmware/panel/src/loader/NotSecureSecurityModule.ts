export default function (prevAccessToken: number, pin: number) {
    let pinCodeTokenMix = 0
    let prevAccessTokenBinaryString = prevAccessToken.toString(2)

    for (let i = 0; i < prevAccessTokenBinaryString.length; i++) {
        pinCodeTokenMix += (prevAccessTokenBinaryString.charAt(i) == "0" ? 0 : 1)^pin << i;
    }

    pinCodeTokenMix ^= pin << pinCodeTokenMix&prevAccessToken
    pinCodeTokenMix = (~pin) ^ pinCodeTokenMix

    pinCodeTokenMix *= pinCodeTokenMix

    // Get the first 16 bits out of this
    let pinCodeTokenMixBinaryString = pinCodeTokenMix.toString(2)
    let first16bits = 0
    for (let i = 0; i < Math.min(pinCodeTokenMixBinaryString.length, 16); i++) {
        first16bits += (pinCodeTokenMixBinaryString.charAt(i) == "0" ? 0 : 1) << i;
    }

    return first16bits
}