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
module x3p_string;

import stdlib;
import additive3pp;
import a3p_random;
import xor3pp;
/**
* \endcond
*/

/**
* @file
* \defgroup x3p_string x3p_string.sc
* \defgroup x3p_bl_string x3p_bl_string
* \defgroup x3p_kl_string x3p_kl_string
* \defgroup CRC CRC
* \defgroup CRC2 CRC(0 initial hash)
* \defgroup murmurhashervec murmurHasherVec
* \defgroup countzeroes countZeroes
* \defgroup bl_str bl_str
* \defgroup bl_strisempty bl_strIsEmpty
* \defgroup bl_strdeclassify bl_strDeclassify
* \defgroup bl_strlength bl_strLength
* \defgroup bl_strequals bl_strEquals
* \defgroup bl_strtrim bl_strTrim
* \defgroup findsortingpermutation findSortingPermutation
* \defgroup bl_strcat bl_strCat
* \defgroup bl_streqprefixes bl_strEqPrefixes
* \defgroup zeroextend zeroExtend
* \defgroup bl_strislessthan bl_strIsLessThan
* \defgroup bl_strlevenshtein bl_strLevenshtein
* \defgroup bl_strcontains bl_strContains
* \defgroup bl_strindexof bl_strIndexOf
* \defgroup bl_strhamming bl_strHamming
* \defgroup kl_str kl_str
* \defgroup kl_strdeclassify kl_strDeclassify
* \defgroup kl_strlength kl_strLength
* \defgroup kl_strequals kl_strEquals
* \defgroup kl_strislessthan kl_strIsLessThan
* \defgroup kl_strcat kl_strCat
* \defgroup kl_streqprefixes kl_strEqPrefixes
* \defgroup kl_strcontains kl_strContains
* \defgroup kl_strindexof kl_strIndexOf
* \defgroup kl_strhamming kl_strHamming
* \defgroup kl_strlevenshtein kl_strLevenshtein
*/

/** \addtogroup <x3p_string>
*@{
* @brief Module with string functions
*/

/*******************************************************************************
********************************************************************************
**                                                                            **
**  CRC                                                                       **
**                                                                            **
********************************************************************************
*******************************************************************************/

/** \addtogroup <CRC>
 *  @{
    @note **D** - additive3pp protection domain
    @note Supported types - \ref xor_uint8 "xor_uint8"
 *  @brief Compute CRC hash of the input byte array with given initial hash.
 *  @param input - the input byte vector
 *  @param hash - the initial hash of type \ref xor_uint16 "xor_uint16" or \ref xor_uint32 "xor_uint32"
 */

template <domain D : additive3pp, dim N>
D xor_uint16 CRC16 (D xor_uint8 [[N]] input, D xor_uint16 hash) {
    __syscall ("additive3pp::crc16_xor_vec", __domainid (D), input, hash);
    return hash;
}


template <domain D : additive3pp, dim N>
D xor_uint32 CRC32 (D xor_uint8 [[N]] input, D xor_uint32 hash) {
    __syscall ("additive3pp::crc32_xor_vec", __domainid (D), input, hash);
    return hash;
}

/** @}*/
/** \addtogroup <CRC2>
 *  @{
    @note **D** - additive3pp protection domain
    @note Supported types - \ref xor_uint8 "xor_uint8"
 *  @brief Compute CRC hash of the input byte array with 0 initial hash.
 *  @param input - the input byte vector
 */

template <domain D : additive3pp, dim N>
D xor_uint16 CRC16 (D xor_uint8 [[N]] input) {
    D xor_uint16 hash = 0;
    return CRC16 (input, hash);
}

template <domain D : additive3pp, dim N>
D xor_uint32 CRC32 (D xor_uint8 [[N]] input) {
    D xor_uint32 hash = 0;
    return CRC32 (input, hash);
}

/** @}*/

/************************************************
*************************************************
**********  utility functions for murmur  *******
*************************************************
*************************************************/
/**
* \cond
*/

template <domain D : additive3pp>
D xor_uint32[[1]] bitSumVec(D xor_uint32[[1]] first, uint32[[1]] second){
    return reshare (reshare (first) + second :: D);
}

template <domain D : additive3pp>
D xor_uint32[[1]] bitMultiplyVec(D xor_uint32[[1]] first, uint32[[1]] second) {
  return reshare (reshare (first) * second :: D);
}

template <domain D : additive3pp>
D xor_uint32[[1]] rotBitsLeftVec (D xor_uint32[[1]] bits, int k){
    int[[1]] shifts (size (bits)) = k;
    __syscall ("additive3pp::rotate_left_xor_uint32_vec", __domainid (D), bits, __cref shifts, bits);
    return bits;
}

template <domain D : additive3pp, dim N>
D xor_uint32[[N]] shiftBitsLeftVec (D xor_uint32[[N]] bits, int k) {
    int[[1]] shifts (size (bits)) = k;
    __syscall ("additive3pp::shift_left_xor_uint32_vec", __domainid (D), bits, __cref shifts, bits);
    return bits;
}

template <domain D : additive3pp, dim N>
D xor_uint32[[N]] shiftBitsRightVec (D xor_uint32[[N]] bits, int k) {
    return shiftBitsLeftVec (bits, - k);
}

/**
* \endcond
*/

/********************************************************
*********************************************************
*************      murmurHash              **************
*********************************************************
********************************************************/

/** \addtogroup <murmurhashervec>
*@{
* @brief murmurHash
* @note **D** - additive3pp protection domain
* @note Supported types - \ref xor_uint32 "xor_uint32"
*/

/**
* @param hashee - the string to be encrypted
* @param seed - the encryption seed
* @return returns an encrypted version of **hashee**
* \todo test if murmur is actually implemented correctly
*/

template <domain D : additive3pp>
D xor_uint32[[1]] murmurHasherVec (D xor_uint32[[1]] hashee,  public uint32[[1]] seed) {
    assert (size (hashee) == size (seed));
    uint rows = size (seed);
    uint32 len = 4; // hashee is 4 bytes long

    D xor_uint32[[1]] hash = seed;

    uint32[[1]] cee1Vec (rows) = 0xcc9e2d51;
    uint32[[1]] cee2Vec (rows) = 0x1b873593;
    uint32[[1]] ennVec (rows)  = 0xe6546b64;
    uint32[[1]] x1Vec (rows)   = 0x85ebca6b;
    uint32[[1]] x2Vec (rows)   = 0xc2b2ae35;
    uint32[[1]] fiveVec (rows) = 5;

    // for each fourByteChunk of key {
    D xor_uint32[[1]] k = hashee;
    k = bitMultiplyVec (k, cee1Vec);
    k = rotBitsLeftVec (k, 15);
    k = bitMultiplyVec (k, cee2Vec);

    hash ^= k;
    hash = rotBitsLeftVec (hash,13);
    hash = bitMultiplyVec (hash, fiveVec);
    hash = bitSumVec (hash, ennVec);
    // }

    hash ^= len;
    hash ^= shiftBitsRightVec (hash, 16);
    hash = bitMultiplyVec (hash, x1Vec);
    hash ^= shiftBitsRightVec(hash, 13);
    hash = bitMultiplyVec (hash, x2Vec);
    hash ^= shiftBitsRightVec (hash,16);
    return hash;
}
/** @}*/


/*******************************************************************************
********************************************************************************
**                                                                            **
**  Various bounded length string algorithms                                  **
**                                                                            **
********************************************************************************
*******************************************************************************/
/** \addtogroup <x3p_bl_string>
*@{
* @brief Module with functions for bounded length strings
*/


/** \addtogroup <countzeroes>
 *  @{
    @note **D** - additive3pp protection domain
    @note Supported types - \ref xor_uint8 "xor_uint8"
 *  @brief Count zeroes in the input vector
 *  @param s - a vector of supported type
 *  @return returns the number of 0 bytes in the input vector
 */

template <domain D : additive3pp>
D uint countZeroes (D xor_uint8[[1]] s) {
    // TODO: I think this can be optimized.
    return sum ((uint) (s == 0));
}
/** @}*/
/** \addtogroup <bl_str>
 *  @{
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8"
 *  @brief Convert a string to a vector of type xor_uint8
 */

/**
 * @return returns a bounded length XOR shared \ref string "string" from the public string and the given bound
 * @param n - an \ref uint64 "uint" type bound
 * @param s - a \ref string "string"
 * @pre the size of the given bound is no less than the length of the input string
 * @post the output string is of length n
 * @note the excess bytes in the shared string are placed in the end and are zeroed
 */
template <domain D : additive3pp>
D xor_uint8[[1]] bl_str (string s, uint n) {
    uint8[[1]] bytes = __bytes_from_string (s);
    uint8[[1]] out (n);
    assert (size(bytes) <= n);
    out[:size(bytes)] = bytes;
    return out;
}

/**
 * @param s - a \ref string "string"
 * @return returns a bounded length XOR shared \ref string "string" from the public string
 * @note the bounded length aligns with the actual length, no extra zeroed bytes are added
 */
template <domain D : additive3pp>
D xor_uint8[[1]] bl_str (string s) {
    return __bytes_from_string (s);
}

/** @}*/
/** \addtogroup <bl_strisempty>
 *  @{
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8"
 *  @note this is done by checking if all of the bytes in the string are zeroed
 *  @param s - a vector of supported type
 *  @brief Check if the input string is empty
 *  @return returns **true** if the given known length input \ref string "string" is empty
 */

template <domain D : additive3pp>
D bool bl_strIsEmpty (D xor_uint8[[1]] s) {
   return all (s == 0);
}

/** @}*/
/** \addtogroup <bl_strdeclassify>
 *  @{
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8"
 *  @param ps - a vector of supported type
 *  @brief Function for converting an array of type xor_uint8 to a string
 *  @return returns a declassified bounded length \ref string "string", extra bytes are removed
 */

template <domain D : additive3pp>
string bl_strDeclassify (D xor_uint8[[1]] ps) {
    uint8[[1]] bytes = declassify (ps);
    uint i = 0;
    for (uint n = size (bytes); i < n; ++i) {
        if (bytes[i] == 0)
            break;
    }

    return __string_from_bytes (bytes[:i]);
}


/** @}*/
/** \addtogroup <bl_strlength>
 *  @{
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8"
 *  @param s - a vector of supported type
 *  @brief Function for finding the length of the given input string
 *  @return returns the actual length of the bounded lengthed input \ref string "string"
 */
template <domain D : additive3pp>
D uint bl_strLength (D xor_uint8[[1]] s) {
    return size (s) - countZeroes (s);
}
/** @}*/
/** \addtogroup <bl_strtrim>
 *  @{
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8"
 *  @note this operation leaks the real length of the string, use with care!
 *  @param s - a vector of supported type
 *  @brief Function for trimming a string
 *  @return returns a \ref string "string" with excess bytes removed
 */
template <domain D : additive3pp>
D xor_uint8[[1]] bl_strTrim (D xor_uint8[[1]] s) {
    uint n = size (s) - declassify (countZeroes (s));
    return s[:n];
}

/** @}*/
/** \addtogroup <bl_strequals>
 *  @{
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8"
 *  @param s,t - vectors of supported type
 *  @brief Compare two string with each other
 *  @return returns **true** if the two input strings are equal
 */
template <domain D : additive3pp>
D bool bl_strEquals (D xor_uint8[[1]] s, D xor_uint8[[1]] t) {
    uint n = size (s), m = size (t);

    if (n > m) {
        return bl_strEquals (t, s);
    }

    return all (s == t[:n]) && bl_strIsEmpty (t[n:]);
}
/** @}*/
/** \addtogroup <findsortingpermutation>
 *  @{
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref bool "bool"
 *  @note performs two vectorized multiplications
 *  @param arr - a vector of supported type
 *  @brief Function for finding a stable sorting permutation
 *  @return returns a stable (in a sorting sense) permutation that moves **false** values to end
 */
template <domain D : additive3pp>
D uint[[1]] findSortingPermutation (D bool[[1]] arr) {
    D uint[[1]] vec = (uint) arr;
    D uint[[1]] ivec = 1 - vec;

    uint n = size (arr);

    D uint[[1]] pTrue (n);
    D uint acc = 0;
    for (uint i = 0; i < n; ++i) {
        pTrue[i] = acc;
        acc += vec[i];
    }

    D uint[[1]] pFalse (n);
    acc = n - 1;
    for (uint i = 1; i <= n; ++i) {
        pFalse[n-i] = acc;
        acc -= ivec[n-i];
    }

    return vec * (pTrue - pFalse) + pFalse;
}
/** @}*/
/** \addtogroup <bl_strcat>
 *  @{
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8"
 *  @param s,t - string vectors of supported type
 *  @brief Function for concatenating two strings
 *  @return returns a concatenation of the two input strings
 */
template <domain D : additive3pp>
D xor_uint8[[1]] bl_strCat (D xor_uint8[[1]] s, D xor_uint8[[1]] t) {
    uint n = size (s) + size (t);
    D xor_uint8[[1]] u = cat (s, t);
    D uint[[1]] p = findSortingPermutation (u != 0);

    { // Shuffle the permutation and concatenation using the same key
        D uint8[[1]] key (32);
        u = shuffle (u, key);
        p = shuffle (p, key);
    }

    uint[[1]] pp = declassify (p); // it's safe to declassify a random permutation
    D xor_uint8[[1]] out (n);
    for (uint i = 0; i < n; ++i) {
        out[pp[i]] = u[i];
    }

    return out;
}

/** @}*/
/** \addtogroup <zeroextend>
 *  @{
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8"
 *  @brief Extend an input string with zeroes.
 *  @param s - an input string
 *  @param n - minimum size that we want the resulting string to be
 *  @return  returns a vector that was created by zeroextending **s**
 *  @postcondition size (output vector) >= **n**
 *  @postcondition **s** is a prefix of the output vector
 *  @postcondition only zeroes have been added to the output vector
 *  @postcondition if size ( **s** ) >= **n** then the output vector == **s**
 */
template <domain D : additive3pp>
D xor_uint8[[1]] zeroExtend (D xor_uint8[[1]] s, uint n) {
    uint m = size (s);
    if (n > m) {
        return cat (s, reshape (0, n - m));
    }

    return s;
}

/** @}*/
/** \addtogroup <bl_strislessthan>
 *  @{
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8"
 *  @param s,t - input string vectors of supported type
 *  @brief function for comparing two strings alphabetically
 *  @return returns **true** if s < t
 */

template <domain D : additive3pp>
D bool bl_strIsLessThan (D xor_uint8[[1]] s, D xor_uint8[[1]] t) {
    s = zeroExtend (s, size (t));
    t = zeroExtend (t, size (s));

    D uint nLE = truePrefixLength (s <= t);
    D uint nEQ = truePrefixLength (s == t);
    return nLE > nEQ;
}

/** @}*/
/** \addtogroup <bl_strlevenshtein>
 *  @{
 *  @brief function for finding the edit distance of the two input strings
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8"
 *  @param s,t - input string vectors of supported type
 *  @return returns the edit distance of the two input strings
 *  @note the algorithm is almost identical to the known length one. The only difference is that the result is found somewhere in the middle of the 2D grid, and not in the bottom right corner. To find it we: a) compute a mask that's one in that position and zero otherwise, b) multiply the mask with the grid, and c) return the sum of the result.
 * \todo we can optimize by computing the entire grid and performing a single multiplication
 */
template <domain D : additive3pp>
D uint bl_strLevenshtein (D xor_uint8[[1]] s, D xor_uint8[[1]] t) {
    uint n = size (s),
         m = size (t);

    if (n < m) {
        return bl_strLevenshtein (t, s);
    }

    uint diagCount = n + m;

    D uint result = 0;

    uint[[1]] diagOff (diagCount), diagEnd (diagCount);
    D uint[[1]] indexMask, neqs;
    {
        uint nm = n * m;
        uint[[1]] ridx (nm), cidx (nm);

        for (uint s = 0, count = 0; s < diagCount; ++ s) {
            diagOff[s] = count;
            for (uint i = 0; i < n; ++ i) {
                if (s - i < m) {
                    ridx[count] = i;
                    cidx[count] = s - i;
                    ++ count;
                }
            }

            diagEnd[s] = count;
        }

        { // compute the inequalitied:
            D xor_uint8[[1]] ss (nm);
            D xor_uint8[[1]] ts (nm);

            for (uint x = 0; x < nm; ++ x) {
                ss[x] = s[ridx[x]];
                ts[x] = t[cidx[x]];
            }

            neqs = (uint) (ss != ts);
        }

        { // Compute the index masks:
            D uint r = bl_strLength (s);
            D uint c = bl_strLength (t);
            result = (uint) (r == 0) * c + (uint) (c == 0) * r;
            indexMask = (uint) ((r == ridx + 1) && (c == cidx + 1));
        }
    }

    D uint[[1]] prevDiag (1) = 0;
    D uint[[1]] currDiag ((uint)(m > 0) + (uint)(n > 0)) = 1;

    uint extendStart = (uint) (m > 1), extendEnd = (uint) (n > 1);
    for (uint s = 0; s < diagCount; ++ s) {
        uint len = size (currDiag);
        {
            uint prevDiagLen = size (prevDiag);
            uint startOff = (uint) (prevDiagLen >= len);
            uint endOff = prevDiagLen - (uint) (prevDiagLen > len);
            prevDiag = prevDiag[startOff:endOff];
        }


        if (s + 2 > m) extendStart = 0;
        if (s + 2 > n) extendEnd = 0;

        D uint[[1]] temp (extendStart + (diagEnd[s] - diagOff[s]) + extendEnd) = s + 2;
        D uint[[1]] smallDiag =
            min (neqs[diagOff[s]:diagEnd[s]] + prevDiag,
                 min(1 + currDiag[1:], 1 + currDiag[:len-1]) );

        temp[extendStart:size(temp)-extendEnd] = smallDiag;
        result += sum (smallDiag * indexMask[diagOff[s]:diagEnd[s]]);

        prevDiag = currDiag;
        currDiag = temp;
    }

    return result;
}
/** @}*/
/** \addtogroup <bl_streqprefixes>
 *  @{
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8"
 *  @brief Function for checking whether a pattern is a substring of the input string
 *  @pre the pattern is not longer than the input string
 *  @param str - input string vector of supported type
 *  @param pat - the pattern to look for
 *  @return returns a boolean vector indicating if the pattern is a substring of the input string at that position
 */

template <domain D : additive3pp>
D bool[[1]] bl_strEqPrefixes (D xor_uint8[[1]] str, D xor_uint8[[1]] pat) {
    uint n = size (str), m = size (pat);

    D xor_uint8[[1]] workList (n + m);
    workList[:n] = str;

    D bool[[2]] eqs;
    {
        D xor_uint8[[1]] strs (n*m);
        D xor_uint8[[1]] pats (n*m);
        for (uint i = 0, off = 0; i < n; ++ i) {
            strs[off : off + m] = workList[i : i + m];
            pats[off : off + m] = pat;
            off += m;
        }

        eqs = reshape ((strs == pats) || (pats == 0), n, m);
    }

    D bool[[1]] acc (n) = true;
    for (uint i = 0; i < m; ++ i) {
        acc = acc && eqs[:, i];
    }

    return acc;
}
/** @}*/


/** \addtogroup <bl_strcontains>
 *  @{
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8"
 *  @param str - input string vector of supported type
 *  @param pat - the pattern to look for
 *  @brief function for checking if a string contains the given pattern
 *  @return returns **true** if the given pattern is a substring of the given string
 */
template <domain D : additive3pp>
D bool bl_strContains (D xor_uint8[[1]] str, D xor_uint8[[1]] pat) {
    if (size (str) == 0) {
        return bl_strIsEmpty (pat);
    }

    return any (bl_strEqPrefixes (str, pat));
}
/** @}*/
/** \addtogroup <bl_strindexof>
 *  @{
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8"
 *  @param str - input string vector of supported type
 *  @param pat - the pattern to look for
 *  @brief function for finding the index of a given pattern
 *  @return returns the position where given pattern is contained in the string
 *  @note if the string is not found returns value that is or equal to size (str)
 */

template <domain D : additive3pp>
D uint bl_strIndexOf (D xor_uint8[[1]] str, D xor_uint8[[1]] pat) {
    return truePrefixLength (! bl_strEqPrefixes (str, pat));
}


/** @}*/
/** \addtogroup <bl_strhamming>
 *  @{
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8"
 *  @param s,t - input string vectors of supported type
 *  @brief Function for finding the number of bytes that the two input strings differ in
 *  @return returns the number of bytes that the inputs differ in
 *  @pre bl_strLength(s) == bl_strLength(t), otherwise the behaviour is undefined
 */

template <domain D : additive3pp>
D uint bl_strHamming (D xor_uint8[[1]] s, D xor_uint8[[1]] t) {
    uint k = min (size (s), size (t));
    return sum ((uint) (s[:k] != t[:k]));
}

/** @}*/
/** @}*/

/*******************************************************************************
********************************************************************************
**                                                                            **
**  Various known length string algorithms                                    **
**                                                                            **
********************************************************************************
*******************************************************************************/

/** \addtogroup <x3p_kl_string>
*@{
* @brief Module with functions for known length strings
*/

/** \addtogroup <kl_str>
 *  @{
 *  @note **D** - additive3pp protection domain
 *  @note Private strings are XOR shared to optimize for equality and comparison checks.
 *  @brief Function for constructing a known length string from given public string
 *  @param s - a \ref string "string"
 *  @return XOR shared byte array of length equal to the input public string.
 */

template <domain D : additive3pp>
D xor_uint8[[1]] kl_str (string s) {
    return __bytes_from_string (s);
}

/** @}*/
/** \addtogroup <kl_strdeclassify>
 *  @{
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8"
 *  @param ps - a string vector of supported type
 *  @brief Function for declassifying a given XOR shared byte string.
 *  @return returns a public string
 */

template <domain D : additive3pp>
string kl_strDeclassify (D xor_uint8[[1]] ps) {
    return __string_from_bytes (declassify (ps));
}


/** @}*/
/** \addtogroup <kl_strlength>
 *  @{
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8"
 *  @param str - a string vector of supported type
 *  @brief Function for getting the length of a string
 *  @return returns a length of the string
 */

template <domain D : additive3pp>
uint kl_strLength (D xor_uint8[[1]] str) {
    return size(str);
}

/** @}*/
/** \addtogroup <kl_strequals>
 *  @{
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8"
 *  @brief Function for checking whether two strings are equal
 *  @param s,t - string vectors of supported type
 *  @return returns **true** if the input strings are equal, **false** it they are not
 */


template <domain D : additive3pp>
D bool kl_strEquals (D xor_uint8[[1]] s, D xor_uint8[[1]] t) {
    if (kl_strLength (s) != kl_strLength (t)) {
        return false;
    }

    return all (s == t);
}

/** @}*/
/** \addtogroup <kl_strislessthan>
 *  @{
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8"
 *  @brief Function for comparing two strings alphabetically
 *  @param s,t - string vectors of supported type
 *  @return returns **true** if the first input string is less than the second (in dictionary order)
 */

template <domain D : additive3pp>
D bool kl_strIsLessThan (D xor_uint8[[1]] s, D xor_uint8[[1]] t) {
    uint n = kl_strLength(s), m = kl_strLength(t);
    if (n == m) {
        D uint x = truePrefixLength (s >= t);
        D uint y = truePrefixLength (s == t);
        return x == y && (x != n);
    }
    else
    if (n < m) {
        t = t[:n];
        D uint x = truePrefixLength (s >= t);
        D uint y = truePrefixLength (s == t);
        return x == y;
    }
    else {
        s = s[:m];
        D uint x = truePrefixLength (s <= t);
        D uint y = truePrefixLength (s == t);
        return x != y;
    }
}

/** @}*/
/** \addtogroup <kl_strcat>
 *  @{
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8"
 *  @param s,t - string vectors of supported type
 *  @brief Function for concatenating two strings
 *  @return returns a concatenation of the two input strings
 */

template <domain D : additive3pp>
D xor_uint8[[1]] kl_strCat (D xor_uint8[[1]] s, D xor_uint8[[1]] t) {
    return cat (s, t);
}

/** @}*/
/** \addtogroup <kl_streqprefixes>
 *  @{
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8"
 *  @brief Function for checking whether a pattern is a substring of the input string
 *  @pre the pattern is not longer than the input string
 *  @param str - the input string vector of supported type
 *  @param pat - the substring to look for
 *  @return returns a boolean vector indicating if the pattern is a substring of the input string in that position
 */

template <domain D : additive3pp>
D bool[[1]] kl_strEqPrefixes (D xor_uint8[[1]] str, D xor_uint8[[1]] pat) {
    uint n = kl_strLength(str),
         m = kl_strLength(pat);

    assert (m <= n);

    // Number of m length prefixes in n:
    uint k = m * (n - m + 1);

    D bool [[2]] eqs;
    { // Compute all prefix equalities:
        D xor_uint8 [[1]] suffixes (k);
        D xor_uint8 [[1]] patterns (k);
        for (uint i = m, off = 0; i <= n; ++ i) {
            suffixes[off : off + m] = str[i - m : i];
            patterns[off : off + m] = pat;
            off += m;
        }

        eqs = reshape (suffixes == patterns, n - m + 1, m);
    }

    D bool [[1]] acc (n - m + 1) = true;
    for (uint i = 0; i < m; ++ i) {
        acc = acc && eqs[:, i];
    }

    return acc;
}

/** @}*/
/** \addtogroup <kl_strcontains>
 *  @{
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8"
 *  @brief Function for checking whether a string contains a pattern or not
 *  @pre the needle is not longer than the haystack
 *  @param str - the haystack
 *  @param pat - the needle
 *  @return returns if the needle is found within the haystack
 */

template <domain D : additive3pp>
D bool kl_strContains (D xor_uint8[[1]] str, D xor_uint8[[1]] pat) {
    return any (kl_strEqPrefixes (str, pat));
}

/** @}*/
/** \addtogroup <kl_strindexof>
 *  @{
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8"
 *  @brief Function for finding the index of a given pattern in the input string
 *  @pre the needle is not longer than the haystack
 *  @param str - the haystack
 *  @param pat - the needle
 *  @return returns the position of the needle in the haystack
 *  @return returns size(str) if the pat is not a substring of str
 */

template <domain D : additive3pp>
D uint kl_strIndexOf (D xor_uint8[[1]] str, D xor_uint8[[1]] pat) {
    uint n = size (str),
         m = size (pat);

    assert (m <= n);
    if (m == 0)
        return 0;

    uint k = n - m + 1;
    D bool [[1]] zeros (n);
    zeros[:k] = kl_strEqPrefixes (str, pat);
    return truePrefixLength (! zeros);
}

/** @}*/
/** \addtogroup <kl_strhamming>
 *  @{
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8"
 *  @brief Function for finding the number of bytes two inputs differ in
 *  @param s,t - input string vectors of supported type
 *  @pre the input strings are of equal length
 *  @return returns the number of bytes that the inputs differ in
 */

template <domain D : additive3pp>
D uint kl_strHamming (D xor_uint8[[1]] s, D xor_uint8[[1]] t) {
    assert (size (s) == size (t));
    return sum ((uint) (s != t));
}

/** @}*/
/** \addtogroup <kl_strlevenshtein>
 *  @{
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8"
 *  @note this is the classic dynamic programming implementation of Levenshtein distance that's simply tuned to minimize the number of the "min" syscalls. Naive approach would invoke "min" O(n*m) times for strings of length n and m. The following implementation invokes the syscall O(n+m) times by computing the table diagonal at a time.
 *  @param s,t - input string vectors of supported type
 *  @brief Function for finding the edit distance of two input strings
 *  @return returns the edit distance of the two input strings
 */

template <domain D : additive3pp>
D uint kl_strLevenshtein (D xor_uint8[[1]] s, D xor_uint8[[1]] t) {
    uint n = size (s),
         m = size (t);

    if (n < m) {
        return kl_strLevenshtein (t, s);
    }

    uint diagCount = n + m;

    //
    // We are flattening the 2D grid so that the diagonals of the grid are consecutively in the array.
    // For that we comput the diagonal offsets:
    //   diagOff - offset of i'th diagonal
    //   diagEnd - end of i'th diagonal
    uint[[1]] diagOff (diagCount), diagEnd (diagCount);
    D uint[[1]] neqs;
    {
        uint nm = n * m;
        uint [[1]] ridx (nm), cidx (nm);
        for (uint s = 0, count = 0; s < diagCount; ++s) {
            diagOff[s] = count;
            for (uint i = 0; i < n; ++i) {
                if (s - i < m) {
                    ridx[count] = i;
                    cidx[count] = s - i;
                    ++count;
                }
            }

            diagEnd[s] = count;
        }

        D xor_uint8[[1]] ss (nm);
        D xor_uint8[[1]] ts (nm);

        for (uint i = 0; i < nm; ++i) {
            ss[i] = s[ridx[i]];
            ts[i] = t[cidx[i]];
        }

        neqs = (uint) (ss != ts);
    }

    //
    // Previous and current diagonal of one unit wider and taller table.
    D uint [[1]] prevDiag (1) = 0;
    D uint [[1]] currDiag ((uint)(m > 0) + (uint)(n > 0)) = 1;

    uint extendStart = (uint) (m > 1), extendEnd = (uint) (n > 1);
    for (uint i = 0; i < diagCount; ++i) {
        uint len = size (currDiag);
        {
            // TODO: can this trimming be avoided?
            uint prevDiagLen = size (prevDiag);
            uint startOff = (uint) (prevDiagLen >= len);
            uint endOff = prevDiagLen - (uint) (prevDiagLen > len);
            prevDiag = prevDiag[startOff:endOff];
        }

        if (i + 2 > m) extendStart = 0;
        if (i + 2 > n) extendEnd = 0;

        D uint[[1]] temp (extendStart + (diagEnd[i] - diagOff[i]) + extendEnd) = i + 2;
        temp[extendStart:size(temp)-extendEnd] =
            min (neqs[diagOff[i]:diagEnd[i]] + prevDiag,
                 min(1 + currDiag[1:], 1 + currDiag[:len-1]) );

        prevDiag = currDiag;
        currDiag = temp;
    }

    return prevDiag[0];
}
/** @}*/
/** @}*/
/** @}*/
