export const nextToken = (prevAccessToken: number, pin: number) => {
    return (2 + (prevAccessToken ^ pin)) & 0xFFFF;
}