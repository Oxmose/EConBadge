package dev.olsontek.econbadge.btserial
/* Code from https://github.com/harryjph/android-bluetooth-serial
 * Slightly updated (less than 10 lines) to send ByteArray instead of strings
 */
class BluetoothConnectException(cause: Throwable) : Exception(cause)
