package dev.olsontek.econbadge.btserial
/* Code from https://github.com/harryjph/android-bluetooth-serial
 * Slightly updated (less than 10 lines) to send ByteArray instead of strings
 */

interface SimpleBluetoothDeviceInterface {
    /**
     * @return The BluetoothSerialDevice instance that the interface is wrapping.
     */
    val device: BluetoothSerialDevice

    fun sendMessage(message: ByteArray)

    /**
     * Set all of the listeners for the interfact
     *
     * @param messageReceivedListener Receive message callback
     * @param messageSentListener Send message callback (indicates that a message was successfully sent)
     * @param errorListener Error callback
     */
    fun setListeners(messageReceivedListener: OnMessageReceivedListener?,
                     messageSentListener: OnMessageSentListener?,
                     errorListener: OnErrorListener?)

    /**
     * Set the message received listener
     *
     * @param listener Receive message callback
     */
    fun setMessageReceivedListener(listener: OnMessageReceivedListener?)

    /**
     * Set the message sent listener
     *
     * @param listener Send message callback (indicates that a message was successfully sent)
     */
    fun setMessageSentListener(listener: OnMessageSentListener?)

    /**
     * Set the error listener
     *
     * @param listener Error callback
     */
    fun setErrorListener(listener: OnErrorListener?)

    interface OnMessageReceivedListener {
        fun onMessageReceived(message: ByteArray)
    }

    interface OnMessageSentListener {
        fun onMessageSent(message: ByteArray)
    }

    interface OnErrorListener {
        fun onError(error: Throwable)
    }
}
