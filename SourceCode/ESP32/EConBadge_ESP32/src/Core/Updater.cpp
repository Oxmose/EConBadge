/*******************************************************************************
 * @file Updater.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 24/02/2025
 *
 * @version 2.0
 *
 * @brief This file defines updater service.
 *
 * @details This file defines updater service. The updater allows to update the
 * EConBadge over the air (OTA).
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <cstdint>          /* Generic types */
#include <HWMgr.h>          /* Hardware layer */
#include <Update.h>         /* ESP32 Update manager */
#include <Logger.h>         /* System logger */
#include <version.h>        /* Versionning information */
#include <Storage.h>        /* Storage service */
#include <BlueToothMgr.h>   /* Bluetooth manager */
#include <mbedtls/sha256.h> /* Checksum functions */
#include <mbedtls/pk.h>     /* Signature functions */

/* Header File */
#include <Updater.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define UPDATE_BUFFER_SIZE 16384
#define UPDATE_MAGIC 0xECB0C0DE
#define REQUEST_TIMEOUT 5000 /* 5 seconds */


/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */


/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/************************* Imported global variables **************************/
/* None */

/************************* Exported global variables **************************/
/* None */

/************************** Static global variables ***************************/
/** @brief Stores the signature public key for update verification. */
const char* skpPublicKey = "-----BEGIN PUBLIC KEY-----\n"
    "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAjMJpqjVljUnP922JYoKA\n"
    "TE1FyqnRRCnikV1iJ3a/F2jlsF2VZIPcbqIrIcnatvOIvjla4QGHbyJI4QbQzAIm\n"
    "+PxPP7bjpOTxFC5IR2j3gIPM7SbZDA82uUQ5UBcupka+Ke4xew1yBG/ei7cx4f0I\n"
    "us52cFkR4OQXfI7RDxzSYwlC3Ucfhu9R4xgOD5jFs34UVMELbeI12SvQ7b4CdTdk\n"
    "XklNTDjv25Su7GraR/9MyHppNTJSyWk7w5tWTKQc6uBOmK/Y8+gp8/WWlu6nktFT\n"
    "ywYSb0PW7mCEww+VedW2Z7Y8a6Go9oO7a3pAUeNIo+pnfrf/UgVMFyqLbhVCCJd6\n"
    "AQIDAQAB\n"
    "-----END PUBLIC KEY-----";

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * CLASS METHODS
 ******************************************************************************/

Updater::Updater(BluetoothManager * pBtMgr)
{
    pBtMgr_   = pBtMgr;
    progress_ = 0;
    pCommandResponse_ = nullptr;
}

uint8_t Updater::GetProgress(void) const
{
    return progress_;
}

void Updater::RequestUpdate(const uint8_t* kpData, SCommandResponse& rReponse)
{
    pCommandResponse_ = &rReponse;

    /* Init the update data */
    memset(&updateHeader_, 0, sizeof(SUpdateHeader));

    LOG_DEBUG("Starting update\n");

    /* Starts the update thread */
    xTaskCreatePinnedToCore(
        UpdateRoutine,
        "UpdateThread",
        4096,
        this,
        20,
        &updateThread_,
        tskNO_AFFINITY
    );

    if(updateThread_ == nullptr)
    {
        rReponse.header.errorCode = ACTION_FAILED;
        rReponse.header.size = 0;
    }
    else
    {
        rReponse.header.errorCode = NO_ERROR;
        rReponse.header.size = 0;
    }
}

bool Updater::DownloadUpdateFile(void)
{
    uint8_t* pBuffer;
    size_t   leftToTransfer;
    size_t   toReceive;
    ssize_t  readBytes;
    ssize_t  wroteBytes;
    size_t   offset;
    Storage* pStore;
    FsFile   updateFile;

    /* Send the ack */
    pBtMgr_->SendCommandResponse(*pCommandResponse_);

    LOG_DEBUG("Waiting for update header\n");
    /* Get the update header */
    readBytes = pBtMgr_->ReceiveData(
        (uint8_t*)&updateHeader_,
        sizeof(SUpdateHeader),
        REQUEST_TIMEOUT
    );

    if(readBytes != sizeof(SUpdateHeader))
    {
        pCommandResponse_->header.errorCode = INVALID_PARAM;
        pCommandResponse_->header.size = 0;
        return false;
    }

    /* Check the header magic */
    if(updateHeader_.magic != UPDATE_MAGIC ||
       strcmp(updateHeader_.compatHw, PROTO_REV) != 0)
    {
        pCommandResponse_->header.errorCode = INVALID_PARAM;
        pCommandResponse_->header.size = 0;
        return false;
    }

    /* Open the update file */
    pStore = Storage::GetInstance();
    pStore->Remove(UPDATE_FILE_PATH);
    updateFile = pStore->Open(UPDATE_FILE_PATH, FILE_WRITE);
    if(!updateFile)
    {
        pCommandResponse_->header.errorCode = FILE_NOT_FOUND;
        pCommandResponse_->header.size = 0;
        return false;
    }

    /* Allocate memory */
    pBuffer = new uint8_t[UPDATE_BUFFER_SIZE];
    if(pBuffer == nullptr)
    {
        updateFile.close();
        pStore->Remove(UPDATE_FILE_PATH);
        pCommandResponse_->header.errorCode = NO_MORE_MEMORY;
        pCommandResponse_->header.size = 0;

        return false;
    }

    /* Ready to receive */
    pCommandResponse_->header.errorCode = NO_ERROR;
    pCommandResponse_->header.size = 0;
    pBtMgr_->SendCommandResponse(*pCommandResponse_);

    leftToTransfer = updateHeader_.size;
    while(leftToTransfer > 0)
    {
        toReceive = MIN(leftToTransfer, UPDATE_BUFFER_SIZE);

        LOG_DEBUG("Downloading Update File. Left: %d\n", leftToTransfer);

        readBytes = pBtMgr_->ReceiveData(
            pBuffer,
            toReceive,
            REQUEST_TIMEOUT
        );
        if(readBytes > 0)
        {

            /* Write the update file */
            offset = 0;
            do
            {
                wroteBytes = updateFile.write(pBuffer + offset, readBytes);
                if(wroteBytes < 0)
                {
                    delete[] pBuffer;
                    updateFile.close();
                    pStore->Remove(UPDATE_FILE_PATH);
                    pCommandResponse_->header.errorCode = WRITE_FILE_FAILED;
                    pCommandResponse_->header.size = 0;
                    return false;
                }
                offset += wroteBytes;
                readBytes -= wroteBytes;
                leftToTransfer -= wroteBytes;
            } while(readBytes != 0);

            SetProgress(
                UPDATE_DOWNLOAD,
                100 - (leftToTransfer * 100 / updateHeader_.size)
            );
        }
        else
        {
            delete[] pBuffer;
            updateFile.close();
            pStore->Remove(UPDATE_FILE_PATH);
            pCommandResponse_->header.errorCode = TRANS_RECV_FAILED;
            pCommandResponse_->header.size = 0;
            return false;
        }
    }

    updateFile.close();
    delete[] pBuffer;

    return true;
}

bool Updater::CheckUpdateFile(void)
{
    uint8_t* pBuffer;
    size_t   toRead;
    size_t   leftToRead;
    ssize_t  readBytes;
    Storage* pStore;
    FsFile   updateFile;
    uint8_t  digest[32];
    char     hexDigest[65];
    char*    cursor;
    bool     hashMatch;
    int32_t  retCode;

    mbedtls_sha256_context shaCtx;
    mbedtls_pk_context     pkCtx;

    /* Check the header magic */
    if(updateHeader_.magic != UPDATE_MAGIC ||
        strcmp(updateHeader_.compatHw, PROTO_REV) != 0)
    {
        pCommandResponse_->header.errorCode = INVALID_PARAM;
        pCommandResponse_->header.size = 0;
        return false;
    }

    /* Open the update file */
    pStore = Storage::GetInstance();
    updateFile = pStore->Open(UPDATE_FILE_PATH, FILE_READ);
    if(!updateFile)
    {
        pCommandResponse_->header.errorCode = FILE_NOT_FOUND;
        pCommandResponse_->header.size = 0;
        return false;
    }

    /* Allocate memory */
    pBuffer = new uint8_t[UPDATE_BUFFER_SIZE];
    if(pBuffer == nullptr)
    {
        updateFile.close();
        pCommandResponse_->header.errorCode = NO_MORE_MEMORY;
        pCommandResponse_->header.size = 0;
        return false;
    }

    /* Start the SHA engine */

    mbedtls_sha256_init(&shaCtx);
    mbedtls_sha256_starts(&shaCtx, false);

    leftToRead = updateHeader_.size;
    while(leftToRead > 0)
    {
        toRead = MIN(leftToRead, UPDATE_BUFFER_SIZE);

        LOG_DEBUG("Checking Update File. Left: %d\n", leftToRead);
        readBytes = updateFile.read(pBuffer, toRead);
        if(readBytes > 0)
        {
            /* Add to the checksum */
            mbedtls_sha256_update(&shaCtx, pBuffer, readBytes);

            leftToRead -= readBytes;
            SetProgress(
                UPDATE_VERIFY,
                100 - (leftToRead * 100 / updateHeader_.size)
            );
        }
        else if(readBytes == 0)
        {
            delete[] pBuffer;
            updateFile.close();
            pCommandResponse_->header.errorCode = READ_FILE_FAILED;
            pCommandResponse_->header.size = 0;
            return false;
        }
    }

    mbedtls_sha256_finish(&shaCtx, digest);

    updateFile.close();
    delete[] pBuffer;

    hashMatch = true;
    for(toRead = 0; toRead < 32; ++toRead)
    {
        snprintf(hexDigest + toRead * 2, 3, "%02X", digest[toRead]);

    }
    LOG_INFO("New firmware hash: %s\n", hexDigest);
    for(toRead = 0; toRead < 32; ++toRead)
    {
        snprintf(
            hexDigest + toRead * 2,
            3,
            "%02X",
            updateHeader_.checksum[toRead]
        );
        /* Perform the hash check */
        if(digest[toRead] != updateHeader_.checksum[toRead])
        {
            hashMatch = false;
        }
    }
    LOG_INFO("Expected hash: %s\n", hexDigest);

    if(!hashMatch)
    {
        pCommandResponse_->header.errorCode = CORRUPTED_DATA;
        pCommandResponse_->header.size = 0;
        return false;
    }

    /* Check signature */
    mbedtls_pk_init(&pkCtx);
    retCode = mbedtls_pk_parse_public_key(
        &pkCtx,
        (uint8_t*)skpPublicKey,
        strlen(skpPublicKey) + 1
    );
    if(retCode != 0)
    {
        LOG_ERROR("Failed to load public key, error: %d\n", retCode);
        pCommandResponse_->header.errorCode = ACTION_FAILED;
        pCommandResponse_->header.size = 0;
        return false;
    }

    LOG_INFO("Expected signature: \n");
    cursor = hexDigest;
    for(toRead = 0; toRead < 256; ++toRead)
    {
        if(toRead % 64 == 0 && toRead != 0)
        {
            LOG_INFO("%s\n", hexDigest);
            cursor = hexDigest;
        }
        snprintf(
            cursor + ((toRead * 2) % 128),
            3,
            "%02X",
            updateHeader_.signature[toRead]
        );

    }
    LOG_INFO("%s\n", hexDigest);

    retCode = mbedtls_pk_verify(
        &pkCtx,
        MBEDTLS_MD_SHA256,
        updateHeader_.checksum,
        sizeof(updateHeader_.checksum),
        updateHeader_.signature,
        sizeof(updateHeader_.signature)
    );
    if(retCode != 0)
    {
        LOG_INFO("Invalid firmware signature\n");
        pCommandResponse_->header.errorCode = CORRUPTED_DATA;
        pCommandResponse_->header.size = 0;
        return false;
    }
    else
    {
        LOG_INFO("Valid firmware signature\n");
    }

    return true;
}

void Updater::ApplyUpdate(void)
{
    uint8_t* pBuffer;
    size_t   toRead;
    size_t   leftToRead;
    ssize_t  readBytes;
    ssize_t  updatedBytes;
    Storage* pStore;
    FsFile   updateFile;


    /* Open the update file */
    pStore = Storage::GetInstance();
    updateFile = pStore->Open(UPDATE_FILE_PATH, FILE_READ);
    if(!updateFile)
    {
        pCommandResponse_->header.errorCode = FILE_NOT_FOUND;
        pCommandResponse_->header.size = 0;
        return;
    }

    /* Allocate memory */
    pBuffer = new uint8_t[UPDATE_BUFFER_SIZE];
    if(pBuffer == nullptr)
    {
        updateFile.close();
        pCommandResponse_->header.errorCode = NO_MORE_MEMORY;
        pCommandResponse_->header.size = 0;
        return;
    }

    if(!update_.begin(updateHeader_.size))
    {
        delete[] pBuffer;
        updateFile.close();
        pCommandResponse_->header.errorCode = ACTION_FAILED;
        pCommandResponse_->header.size = 0;
        return;
    }

    leftToRead = updateHeader_.size;
    while(leftToRead > 0)
    {
        toRead = MIN(leftToRead, UPDATE_BUFFER_SIZE);

        readBytes = updateFile.read(pBuffer, toRead);
        if(readBytes > 0)
        {
            updatedBytes = update_.write(pBuffer, readBytes);
            if(updatedBytes != readBytes)
            {
                update_.abort();
                updateFile.close();
                delete[] pBuffer;

                pCommandResponse_->header.errorCode = CORRUPTED_DATA;
                pCommandResponse_->header.size = 0;
                return;

            }
            leftToRead -= readBytes;
            SetProgress(
                UPDATE_APPLY,
                100 - (leftToRead * 100 / updateHeader_.size)
            );
        }
        else if(readBytes == 0)
        {
            update_.abort();
            updateFile.close();
            delete[] pBuffer;
            pCommandResponse_->header.errorCode = READ_FILE_FAILED;
            pCommandResponse_->header.size = 0;
            return;
        }
        LOG_DEBUG("Loading Update File. Left: %d | Read %d\n", leftToRead, readBytes);
    }

    if(!update_.end())
    {
        pCommandResponse_->header.errorCode = ACTION_FAILED;
        pCommandResponse_->header.size = 0;
    }
    else
    {
        pCommandResponse_->header.errorCode = NO_ERROR;
        pCommandResponse_->header.size = 0;
    }
    updateFile.close();
    delete[] pBuffer;

    SetProgress(UPDATE_APPLY, 100);

    pBtMgr_->SendCommandResponse(*pCommandResponse_);

    /* Delay and restart */
    HWManager::DelayExecUs(100000);
    ESP.restart();
}

void Updater::SetProgress(const EUpdateStep kStep, const uint8_t kProgress)
{
    switch(kStep)
    {
        case UPDATE_DOWNLOAD:
            progress_ = kProgress / 3;
            break;
        case UPDATE_VERIFY:
            progress_ = 33 + kProgress / 3;
            break;
        case UPDATE_APPLY:
            if(kProgress == 100)
            {
                progress_ = 100;
            }
            else
            {
                progress_ = 64 + kProgress / 3;
            }
        default:
            break;
    }
}

void Updater::UpdateRoutine(void* pUpdaterParam)
{
    Updater* pUpdater;

    pUpdater = (Updater*)pUpdaterParam;

    /* Download the update file */
    if(!pUpdater->DownloadUpdateFile())
    {
        vTaskDelete(nullptr);
        return;
    }

    /* Validate the update file */
    if(!pUpdater->CheckUpdateFile())
    {
        vTaskDelete(nullptr);
        return;
    }

    /* Apply the update */
    pUpdater->ApplyUpdate();

    /* Exit */
    vTaskDelete(nullptr);
}