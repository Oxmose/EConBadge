import datetime
import shutil
from Crypto.PublicKey import RSA
from Crypto.Signature import PKCS1_v1_5
from Crypto.Hash import  SHA256

Import("env")

FILENAME_BUILDNO = 'versioning'
FILENAME_VERSION_H = 'include/version.h'
BUFFER_SIZE = 65536
RSA_KEY_PATH = "C:\\Users\\Alexy\\Documents\\sshKeys\\ecbKey.key"

MAJOR = 0
MINOR = 3
version = 'v0.3.'
build_no = 0

FULL_VERSION = ""

def update_versioning():
    global build_no
    global FULL_VERSION

    build_no = 0
    try:
        with open(FILENAME_BUILDNO) as f:
            build_no = int(f.readline()) + 1
    except:
        print('Starting build number from 1..')
        build_no = 1
    with open(FILENAME_BUILDNO, 'w+') as f:
        f.write(str(build_no))
        print('Build number: {}'.format(build_no))

    hf = """
    #ifndef BUILD_NUMBER
        #define BUILD_NUMBER "{}"
    #endif
    #ifndef VERSION
        #define VERSION "{}.{}.{} {}"
    #endif
    #ifndef VERSION_SHORT
        #define VERSION_SHORT "{}.{}.{}"
    #endif
    """.format(
        build_no,
        MAJOR, MINOR, build_no, str(datetime.datetime.now())[:-7],
        MAJOR, MINOR, build_no)

    FULL_VERSION = "{}.{}.{} {}".format(MAJOR, MINOR, build_no, str(datetime.datetime.now())[:-7])
    with open(FILENAME_VERSION_H, 'w+') as f:
        f.write(hf)

def post_program_action(source, target, env):
    sha256 = SHA256.new()
    path = target[0].get_abspath()

    print("Calculating firmware checksum for {}".format(path))

    with open(path, "rb") as file:
        while True:
            data = file.read(BUFFER_SIZE)
            if not data:
                break

            sha256.update(data)

    print("Firmware checksum: {0}".format(sha256.hexdigest()))

    key = None
    with open(RSA_KEY_PATH) as keyfile:
        key = keyfile.read()

    if key == None:
        print("Failed to load RSA key")
        exit -1

    privKey = RSA.importKey(key)

    # Sign firmware
    signer = PKCS1_v1_5.new(privKey)
    signature = signer.sign(sha256)

    print("Firmware signature: {0}".format(signature.hex()))

    # Verify
    signer = PKCS1_v1_5.new(privKey.publickey())
    try:
        signer.verify(sha256, signature)
        print("Signature OK")
    except(ValueError, TypeError):
        print("Signature failed")
        exit -1

    with open(path + ".hash", "wb") as file:
        file.write(sha256.digest())
    with open(path + ".sig", "wb") as file:
        file.write(signature)

    with open(path + ".version", "w") as file:
        file.write(FULL_VERSION)
        file.write(" ")
        file.write(env.GetProjectOption("hardware_compat"))

    shutil.copyfile(path + ".hash", "firmware_latest.bin.hash")
    shutil.copyfile(path + ".sig", "firmware_latest.bin.sig")
    shutil.copyfile(path + ".version", "firmware_latest.bin.version")
    shutil.copyfile(path, "firmware_latest.bin")



update_versioning()

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", post_program_action)
env.Replace(PROGNAME="firmware_{}_{}_{}".format(MAJOR, MINOR, build_no))

# Import("env")
# env.Execute("python -m pip install pycryptodome pycryptodomex")