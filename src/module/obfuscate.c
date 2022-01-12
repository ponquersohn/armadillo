#include <crypto/skcipher.h>
#include <linux/scatterlist.h>
#include <linux/random.h>

#include "obfuscate.h"
#include "module.h"

// based on the example from: https://www.kernel.org/doc/html/v4.17/crypto/api-samples.html
struct skcipher_def
{
    struct scatterlist sg;
    struct crypto_skcipher *tfm;
    struct skcipher_request *req;
    struct crypto_wait wait;
};

struct skcipher_def sk;
struct crypto_skcipher *skcipher = NULL;
struct skcipher_request *req = NULL;
char *scratchpad = NULL;
char *ivdata = NULL;
unsigned char key[32] = {OBFUSCATION_KEY};
int use_encryption = 1;

int init_obfuscation(struct skcipher_def *sk)
{
    APRINTK_NOLOCK("armadillo: initiating obfuscation\n");
    skcipher = crypto_alloc_skcipher("cbc-aes-aesni", 0, 0);
    if (IS_ERR(skcipher))
    {
        APRINTK_NOLOCK("armadillo: could not allocate skcipher handle: crypto_alloc_skcipher\n");
        return PTR_ERR(skcipher);
    }

    req = skcipher_request_alloc(skcipher, GFP_KERNEL);
    if (!req)
    {
        APRINTK_NOLOCK("armadillo: could not allocate skcipher request: skcipher_request_alloc\n");
        return -ENOMEM;
    }

    /* IV will be random */
    ivdata = kmalloc(16, GFP_KERNEL);
    if (!ivdata)
    {
        APRINTK_NOLOCK("armadillo: could not allocate ivdata: kmalloc\n");
        return -ENOMEM;
    }
    get_random_bytes(ivdata, 16);

    sk->tfm = skcipher;
    sk->req = req;

    return 1;
}

int obfuscate(char *plane, char *obfuscated)
{
    int ret = -EFAULT;
    APRINTK_NOLOCK(KERN_INFO "armadillo: attempting to obfuscate passphrase");

    // APRINTK_NOLOCK(KERN_INFO "obfuscate: plane: %s", plane);

    if (ivdata == NULL)
    {
        ret = init_obfuscation(&sk);
        if (!ret)
        {
            APRINTK_NOLOCK(KERN_INFO "armadillo: Unable to use encryption - falling back to unencrypted passwords.");
            use_encryption = 0;
            // free what we used before
            if (skcipher)
                crypto_free_skcipher(skcipher);
            if (req)
                skcipher_request_free(req);
            if (ivdata)
                kfree(ivdata);
        }
        use_encryption = 1;
        APRINTK_NOLOCK(KERN_INFO "armadillo: Obfuscation initialized.");
    }

    if (use_encryption)
    {
        APRINTK_NOLOCK(KERN_INFO "armadillo: Attempting to obfuscate passphrase.");
        // copy all the bytes from plane to obfuscated
        memcpy(obfuscated, plane, ARMADILLO_MAX_PASS_LENGTH_TERMINATED);
        /* We encrypt one block */
        sg_init_one(&sk.sg, obfuscated, 16);
        skcipher_request_set_crypt(req, &sk.sg, &sk.sg, 16, ivdata);
        crypto_init_wait(&sk.wait);
        ret = crypto_wait_req(crypto_skcipher_encrypt(sk.req), &sk.wait);
        if (ret)
        {
            APRINTK_NOLOCK(KERN_ERR "armadillo: Unable to obfuscate passphrase. ret: %d", ret);
        }
        return 0;
    }
    else
    {
        APRINTK_NOLOCK(KERN_ERR "armadillo: Unable to obfuscate passphrase. ret: %d", ret);
        strncpy(obfuscated, plane, ARMADILLO_MAX_PASS_LENGTH_TERMINATED);
        if (!obfuscated)
        {
            APRINTK_NOLOCK(KERN_ERR "armadillo: Unable to copy obfuscated passphrase.");
            return -EFAULT;
        }
        return 0;
    }
}

int deobfuscate(char *obfuscated, char *plane)
{
    int ret = -EFAULT;

    if (use_encryption)
    {
        APRINTK_NOLOCK(KERN_INFO "armadillo: Attempting to deobfuscate passphrase.");
        // copy all the bytes from plane to obfuscated
        memcpy(plane, obfuscated, ARMADILLO_MAX_PASS_LENGTH_TERMINATED);
        /* We encrypt one block */
        sg_init_one(&sk.sg, obfuscated, 16);
        skcipher_request_set_crypt(req, &sk.sg, &sk.sg, 16, ivdata);
        crypto_init_wait(&sk.wait);
        ret = crypto_wait_req(crypto_skcipher_decrypt(sk.req), &sk.wait);
        if (ret)
        {
            APRINTK_NOLOCK(KERN_ERR "Unable to deobfuscate passphrase. ret: %d", ret);
            return -EFAULT;
        }
        // will be encrypting the password
        return 0;
    }
    else
    {
        strncpy(plane, obfuscated, ARMADILLO_MAX_PASS_LENGTH_TERMINATED);
        if (!plane)
        {
            APRINTK_NOLOCK(KERN_ERR "armadillo: Unable to copy deobfuscated passphrase.");
            return -EFAULT;
        }
        return 0;
    }
}