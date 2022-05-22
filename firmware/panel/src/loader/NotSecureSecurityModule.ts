export default function (prevAccessToken: number, pin: number) {
    return (2+(prevAccessToken ^ pin)) & 0xFFFF;
}