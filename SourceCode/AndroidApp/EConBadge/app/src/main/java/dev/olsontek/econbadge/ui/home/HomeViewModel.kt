package dev.olsontek.econbadge.ui.home

import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel

class HomeViewModel : ViewModel() {
    private val ecbIdentifier = MutableLiveData<String>()
    val ecbIdentifierText: LiveData<String> = ecbIdentifier

    private val ecbSoftwareVersion = MutableLiveData<String>()
    val ecbSoftwareVersionText: LiveData<String> = ecbSoftwareVersion

    private val ecbHardwareVersion = MutableLiveData<String>()
    val ecbHardwareVersionText: LiveData<String> = ecbHardwareVersion

    private val ecbToken = MutableLiveData<String>()
    val ecbTokenText: LiveData<String> = ecbToken

    private val ecbOwner = MutableLiveData<String>()
    val ecbOwnerText: LiveData<String> = ecbOwner

    private val ecbContact = MutableLiveData<String>()
    val ecbContactText: LiveData<String> = ecbContact



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