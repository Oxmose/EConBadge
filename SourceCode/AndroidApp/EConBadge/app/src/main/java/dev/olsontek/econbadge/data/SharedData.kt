package dev.olsontek.econbadge.data

import android.content.Context
import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.Preferences
import androidx.datastore.preferences.core.edit
import androidx.datastore.preferences.core.stringPreferencesKey
import androidx.datastore.preferences.preferencesDataStore
import kotlinx.coroutines.flow.first

/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/

/* ECB settings name in the DataStore environment */
private const val ECB_SETTINGS_DATASTORE_NAME = "ecbSettings"

/***************************************************************************************************
 * MAIN CLASS
 **************************************************************************************************/
class SharedData private constructor() {

    /***********************************************************************************************
     * PUBLIC TYPES AND ENUMERATIONS
     **********************************************************************************************/
    /* Singleton object for the bluetooth manager */
    companion object {
        /* Singleton instance */
        @Volatile
        private var instance: SharedData? = null

        /* Create the singleton instance */
        fun getInstance() =
            instance ?: synchronized(this) {
                instance ?: SharedData().also {
                    instance = it
                }
            }
    }

    /***********************************************************************************************
     * PRIVATE TYPES AND ENUMERATIONS
     **********************************************************************************************/
    /* None */

    /***********************************************************************************************
     * PRIVATE ATTRIBUTES
     **********************************************************************************************/
    /* The dataStore instance */
    private val Context.dataStore: DataStore<Preferences> by preferencesDataStore(
        name = ECB_SETTINGS_DATASTORE_NAME
    )

    /* Storage keys */
    private val identifierKey = stringPreferencesKey("ecbIdentifier")
    private val tokenKey = stringPreferencesKey("ecbToken")

    /***********************************************************************************************
     * PRIVATE METHODS
     **********************************************************************************************/
    /* None */

    /***********************************************************************************************
     * PUBLIC METHODS
     **********************************************************************************************/
    /* Returns the ECB identifier (HWUID) */
    suspend fun getECBIdentifier(context: Context)  : String {
        return context.dataStore.data.first()[identifierKey] ?: ""
    }

    /* Returns the ECB Bluetooth Token */
    suspend fun getECBToken(context: Context)  : String {
        return context.dataStore.data.first()[tokenKey] ?: ""
    }

    /* Sets the ECB identifier (HWUID) */
    suspend fun setECBIdentifier(context: Context, value: String) {
        context.dataStore.edit { settings ->
            settings[identifierKey] = value
        }
    }

    /* Sets the ECB Bluetooth Token */
    suspend fun setECBToken(context: Context, value: String) {
        context.dataStore.edit { settings ->
            settings[tokenKey] = value
        }
    }
}