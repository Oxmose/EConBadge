package dev.olsontek.econbadge.ui.login

import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import dev.olsontek.econbadge.MainActivity
import dev.olsontek.econbadge.R
import dev.olsontek.econbadge.data.SharedData
import dev.olsontek.econbadge.databinding.ActivityConnectBinding
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking

/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/
/* None */

/***************************************************************************************************
 * MAIN CLASS
 **************************************************************************************************/
class ConnectActivity : AppCompatActivity() {

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
    /* Layout element binding */
    private lateinit var binding: ActivityConnectBinding

    /***********************************************************************************************
     * PRIVATE METHODS
     **********************************************************************************************/
    /* None */

    /***********************************************************************************************
     * PUBLIC METHODS
     **********************************************************************************************/
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val sharedData = SharedData.getInstance()

        /* Bindings */
        binding = ActivityConnectBinding.inflate(layoutInflater)
        setContentView(binding.root)

        /* Hide title bar */
        supportActionBar?.hide()

        /* Get components */
        val ecbIdentifier = binding.ecbIdentifierEditText
        val ecbToken = binding.ecbTokenEditText
        val connect = binding.connectButton

        connect.isEnabled = false
        connect.setOnClickListener {
            var isValid = true
            if (ecbIdentifier.text?.isEmpty() == true) {
                ecbIdentifier.error = getString(R.string.invalid_identifier)
                isValid = false
            }
            if (ecbToken.text?.isEmpty() == true) {
                ecbToken.error = getString(R.string.invalid_token)
                isValid = false
            }

            if (isValid) {
                runBlocking {
                    launch {
                        /* Write to shared data */
                        sharedData.setECBIdentifier(
                            applicationContext,
                            ecbIdentifier.text.toString()
                        )
                        sharedData.setECBToken(applicationContext, ecbToken.text.toString())

                        /* Start the main activity */
                        startActivity(Intent(applicationContext, MainActivity::class.java))
                        finish()
                    }
                }
            }
        }

        /* Setup the inputs */
        runBlocking {
            launch {
                val ecbSavedId = sharedData.getECBIdentifier(applicationContext)
                if (ecbSavedId.isNotEmpty()) {
                    ecbIdentifier.setText(ecbSavedId)

                    /* Check if we have an error in the bundle that tels us that the id was
                     * incorrect
                     */
                    if (intent.getBooleanExtra("ecbIdentifierIncorrect", false)) {
                        ecbIdentifier.error = getString(R.string.invalid_identifier)
                    }

                }

                val ecbSavedToken = sharedData.getECBToken(applicationContext)
                if (ecbSavedToken.isNotEmpty()) {
                    ecbToken.setText(ecbSavedToken)

                    /* Check if we have an error in the bundle that tels us that the id was
                     * incorrect
                     */
                    if (intent.getBooleanExtra("ecbTokenIncorrect", false)) {
                        ecbToken.error = getString(R.string.invalid_token)
                    }
                }

                connect.isEnabled = true
            }
        }
    }
}