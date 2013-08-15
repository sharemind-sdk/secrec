/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */
/**
* \cond
*/
module x3p_aes;

import stdlib;
import additive3pp;
import a3p_random;
/**
* \endcond
*/
/**
* @file
* \defgroup x3p_aes x3p_aes.sc
* \defgroup aes_genkey aesGenkey
* \defgroup aes_expandkey aesExpandKey
* \defgroup aes_encrypt aesEncryptEcb
* \defgroup aes_decrypt aesDecryptEcb
*/

/** \addtogroup <x3p_aes> 
*@{
* @brief Module with AES128/192/256 functions
*/

/*******************************************************************************
********************************************************************************
**                                                                            **
**  AES128                                                                    **
**                                                                            **
********************************************************************************
*******************************************************************************/

/** \addtogroup <aes_genkey> 
 *  @{
 *  @brief Function for generating a key for AES encryption
 *  @param blocks - an \ref uint64 "uint" type value
 *  @return returns a vector of type \ref xor_uint32 "xor_uint32" with a randomly generated key
 */

/**
*  @pre ( \ref uint64 "uint" max value ) / 4 >= blocks 
*/
template <domain D : additive3pp>
D xor_uint32[[1]] aes128Genkey(uint blocks) {
    assert(UINT64_MAX / 4 >= blocks); // Check for overflow
    D xor_uint32[[1]] r (blocks * 4);
    r = randomize(r);
    return r;
}

/** @}*/
/** \addtogroup <aes_expandkey> 
 *  @{
 *  @brief Function for expanding a randomly generated AES key
 *  @param aeskey - a 1-dimensional array of type \ref xor_uint32 "xor_uint32". See also \ref aes_genkey "aesGenkey"
 *  @return returns a vector of type \ref xor_uint32 "xor_uint32" with an expanded key
 */

/**
*  @pre the size of **aeskey** has to be dividable by 4
*/
template <domain D : additive3pp>
D xor_uint32[[1]] aes128ExpandKey(D xor_uint32[[1]] aeskey) {
    assert((size(aeskey) % 4) == 0);
    D xor_uint32[[1]] expandedKey (size(aeskey) * 11);
    __syscall("additive3pp::aes128_xor_uint32_vec_expand_key", __domainid(D), aeskey, expandedKey);
    return expandedKey;
}

/** @}*/
/** \addtogroup <aes_encrypt> 
 *  @{
 *  @brief Function for encrypting with AES algorithm
 *  @return returns a vector of type \ref xor_uint32 "xor_uint32" with the encrypted values
 */


/**
* @param expandedKey - an aes128 expanded key of type \ref xor_uint32 "xor_uint32". See also \ref aes_genkey "aesGenkey" and \ref aes_expandkey "aesExpandKey"
* @param plainText - a \ref string "string" converted to a \ref xor_uint32 "xor_uint32" vector
* @pre the size of **expandedKey** has to be dividable by 4
* @pre the size of **plainText** has to be dividable by 4
* @pre ( **plainText** / 4 ) == ( size of **expandedKey** ) / (4 * 11)
*/
template <domain D : additive3pp>
D xor_uint32[[1]] aes128EncryptEcb(D xor_uint32[[1]] expandedKey, D xor_uint32[[1]] plainText) {
    assert(size(plainText) > 0);
    assert((size(expandedKey) % 4) == 0);
    assert((size(plainText) % 4) == 0);
    assert((size(plainText) / 4) == (size(expandedKey) / (4 * 11)));
    D xor_uint32[[1]] cipherText (size(plainText));
    __syscall("additive3pp::aes128_xor_uint32_vec", __domainid(D), plainText, expandedKey, cipherText);
    return cipherText;
}

/** @}*/
/** \addtogroup <aes_decrypt> 
 *  @{
 *  @brief Function for decrypting with AES algorithm
 *  @return returns a \ref xor_uint32 "xor_uint32" type vector with the decrypted values
 */

/**
* @param expandedKey - an aes128 expanded key of type \ref xor_uint32 "xor_uint32". See also \ref aes_genkey "aesGenkey" and \ref aes_expandkey "aesExpandKey"
* @param cipherText - an encrypted aes128 cipher of type \ref xor_uint32 "xor_uint32"
*/
template <domain D : additive3pp>
D xor_uint32[[1]] aes128DecryptEcb(D xor_uint32[[1]] expandedKey, D xor_uint32[[1]] cipherText) {
    return aes128EncryptEcb(expandedKey, cipherText);
}

/** @}*/

/*******************************************************************************
********************************************************************************
**                                                                            **
**  AES192                                                                    **
**                                                                            **
********************************************************************************
*******************************************************************************/


/** \addtogroup <aes_genkey> 
 *  @{
 */

/**
*  @pre ( \ref uint64 "uint" max value ) / 6 >= blocks 
*/
template <domain D : additive3pp>
D xor_uint32[[1]] aes192Genkey(uint blocks) {
    assert(UINT64_MAX / 6 >= blocks); // Check for overflow
    D xor_uint32[[1]] r (blocks * 6);
    r = randomize(r);
    return r;
}
/** @}*/
/** \addtogroup <aes_expandkey> 
 *  @{
 */

/**
*  @pre the size of **aeskey** has to be dividable by 6
*/
template <domain D : additive3pp>
D xor_uint32[[1]] aes192ExpandKey(D xor_uint32[[1]] aeskey) {
    assert((size(aeskey) % 6) == 0);
    D xor_uint32[[1]] expandedKey ((size(aeskey) / 6) * 4 * 13);
    __syscall("additive3pp::aes192_xor_uint32_vec_expand_key", __domainid(D), aeskey, expandedKey);
    return expandedKey;
}

/** @}*/
/** \addtogroup <aes_encrypt> 
 *  @{
 */


/**
* @param expandedKey - an aes192 expanded key of type \ref xor_uint32 "xor_uint32". See also \ref aes_genkey "aesGenkey" and \ref aes_expandkey "aesExpandKey"
* @param plainText - a \ref string "string" converted to a \ref xor_uint32 "xor_uint32" vector
* @pre the size of **expandedKey** has to be dividable by 4
* @pre the size of **plainText** has to be dividable by 4
* @pre ( **plainText** / 4 ) == ( size of **expandedKey** ) / (4 * 13)
*/
template <domain D : additive3pp>
D xor_uint32[[1]] aes192EncryptEcb(D xor_uint32[[1]] expandedKey, D xor_uint32[[1]] plainText) {
    assert(size(plainText) > 0);
    assert((size(expandedKey) % 4) == 0);
    assert((size(plainText) % 4) == 0);
    assert((size(plainText) / 4) == (size(expandedKey) / (4 * 13)));
    D xor_uint32[[1]] cipherText (size(plainText));
    __syscall("additive3pp::aes192_xor_uint32_vec", __domainid(D), plainText, expandedKey, cipherText);
    return cipherText;
}

/** @}*/
/** \addtogroup <aes_decrypt> 
 *  @{
 */

/**
* @param expandedKey - an aes192 expanded key of type \ref xor_uint32 "xor_uint32". See also \ref aes_genkey "aesGenkey" and \ref aes_expandkey "aesExpandKey"
* @param cipherText - an encrypted aes192 cipher of type \ref xor_uint32 "xor_uint32"
*/
template <domain D : additive3pp>
D xor_uint32[[1]] aes192DecryptEcb(D xor_uint32[[1]] expandedKey, D xor_uint32[[1]] cipherText) {
    return aes192EncryptEcb(expandedKey, cipherText);
}

/** @}*/

/*******************************************************************************
********************************************************************************
**                                                                            **
**  AES256                                                                    **
**                                                                            **
********************************************************************************
*******************************************************************************/

/** \addtogroup <aes_genkey> 
 *  @{
 */

/**
*  @pre ( \ref uint64 "uint" max value ) / 8 >= blocks 
*/
template <domain D : additive3pp>
D xor_uint32[[1]] aes256Genkey(uint blocks) {
    assert(UINT64_MAX / 8 >= blocks); // Check for overflow
    D xor_uint32[[1]] r (blocks * 8);
    r = randomize(r);
    return r;
}
/** @}*/
/** \addtogroup <aes_expandkey> 
 *  @{
 */

/**
*  @pre the size of **aeskey** has to be dividable by 8
*/
template <domain D : additive3pp>
D xor_uint32[[1]] aes256ExpandKey(D xor_uint32[[1]] aeskey) {
    assert((size(aeskey) % 8) == 0);
    D xor_uint32[[1]] expandedKey ((size(aeskey) / 8) * 4 * 15);
    __syscall("additive3pp::aes256_xor_uint32_vec_expand_key", __domainid(D), aeskey, expandedKey);
    return expandedKey;
}
/** @}*/
/** \addtogroup <aes_encrypt> 
 *  @{
 */


/**
* @param expandedKey - an aes256 expanded key of type \ref xor_uint32 "xor_uint32". See also \ref aes_genkey "aesGenkey" and \ref aes_expandkey "aesExpandKey"
* @param plainText - a \ref string "string" converted to a \ref xor_uint32 "xor_uint32" vector
* @pre the size of **expandedKey** has to be dividable by 4
* @pre the size of **plainText** has to be dividable by 4
* @pre ( **plainText** / 4 ) == ( size of **expandedKey** ) / (4 * 15)
*/
template <domain D : additive3pp>
D xor_uint32[[1]] aes256EncryptEcb(D xor_uint32[[1]] expandedKey, D xor_uint32[[1]] plainText) {
    assert(size(plainText) > 0);
    assert((size(expandedKey) % 4) == 0);
    assert((size(plainText) % 4) == 0);
    assert((size(plainText) / 4) == (size(expandedKey) / (4 * 15)));
    D xor_uint32[[1]] cipherText (size(plainText));
    __syscall("additive3pp::aes256_xor_uint32_vec", __domainid(D), plainText, expandedKey, cipherText);
    return cipherText;
}
/** @}*/
/** \addtogroup <aes_decrypt> 
 *  @{
 */

/**
* @param expandedKey - an aes256 expanded key of type \ref xor_uint32 "xor_uint32". See also \ref aes_genkey "aesGenkey" and \ref aes_expandkey "aesExpandKey"
* @param cipherText - an encrypted aes256 cipher of type \ref xor_uint32 "xor_uint32"
*/
template <domain D : additive3pp>
D xor_uint32[[1]] aes256DecryptEcb(D xor_uint32[[1]] expandedKey, D xor_uint32[[1]] cipherText) {
    return aes256EncryptEcb(expandedKey, cipherText);
}
/** @}*/
/** @}*/
