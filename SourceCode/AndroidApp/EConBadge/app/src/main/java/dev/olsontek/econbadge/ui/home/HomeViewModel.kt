package dev.olsontek.econbadge.ui.home

import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel

/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/
/* None */

/***************************************************************************************************
 * MAIN CLASS
 **************************************************************************************************/
class HomeViewModel : ViewModel() {
    /***********************************************************************************************
     * PUBLIC TYPES AND ENUMERATIONS
     **********************************************************************************************/
    /* None */

    /***********************************************************************************************
     * PRIVATE TYPES AND ENUMERATIONS
     **********************************************************************************************/
    /* None */

    /***********************************************************************************************
     * PRIVATE ATTRIBUTES
     **********************************************************************************************/

    /* ECB identifier */
    private val ecbIdentifier = MutableLiveData<String>()
    val ecbIdentifierText: LiveData<String> = ecbIdentifier

    /* ECB software version */
    private val ecbSoftwareVersion = MutableLiveData<String>()
    val ecbSoftwareVersionText: LiveData<String> = ecbSoftwareVersion

    /* ECB hardware version */
    private val ecbHardwareVersion = MutableLiveData<String>()
    val ecbHardwareVersionText: LiveData<String> = ecbHardwareVersion

    /* Current ECB bluetooth token */
    private val ecbToken = MutableLiveData<String>()
    val ecbTokenText: LiveData<String> = ecbToken

    /* ECB owner text */
    private val ecbOwner = MutableLiveData<String>()
    val ecbOwnerText: LiveData<String> = ecbOwner

    /* ECB owner contact text */
    private val ecbContact = MutableLiveData<String>()
    val ecbContactText: LiveData<String> = ecbContact

    /***********************************************************************************************
     * PRIVATE METHODS
     **********************************************************************************************/
    /* None */

    /***********************************************************************************************
     * PUBLIC METHODS
     **********************************************************************************************/
    fun setIdentifier(value: String) {
        ecbIdentifier.value = value
    }

    fun setSoftwareVersion(value: String) {
        ecbSoftwareVersion.value = value
    }

    fun setHardwareVersion(value: String) {
        ecbHardwareVersion.value = value
    }

    fun setOwner(value: String) {
        ecbOwner.value = value
    }

    fun setContact(value: String) {
        ecbContact.value = value
    }
}