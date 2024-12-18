#include <keystore.h>
#include <primitives/block.h>
#include <util.h>

typedef std::vector<uint8_t> valtype;

bool GetKeyIDFromUTXO(const CTxOut& txout, CKeyID& keyID)
{
    std::vector<valtype> vSolutions;
    txnouttype whichType;
    if (!Solver(txout.scriptPubKey, whichType, vSolutions))
        return false;
    if (whichType == TX_PUBKEY) {
        keyID = CPubKey(vSolutions[0]).GetID();
    } else if (whichType == TX_PUBKEYHASH) {
        keyID = CKeyID(uint160(vSolutions[0]));
    }

    return true;
}

bool SignBlock(CBlock& block, const CKeyStore& keystore)
{
    std::vector<valtype> vSolutions;
    CKeyID keyID;
    if (block.IsProofOfWork()) {
        bool fFoundID = false;
        for (const CTxOut& txout : block.vtx[0]->vout) {
            if (!GetKeyIDFromUTXO(txout, keyID))
                continue;
            fFoundID = true;
            break;
        }
        if (!fFoundID)
            return error("%s: failed to find key for PoW", __func__);
    } else {
        if (!GetKeyIDFromUTXO(block.vtx[1]->vout[1], keyID))
            return error("%s: failed to find key for PoS", __func__);
    }

    CKey key;
    if (!keystore.GetKey(keyID, key))
        return error("%s: failed to get key from keystore", __func__);

    if (!key.Sign(block.GetHash(), block.vchBlockSig))
        return error("%s: failed to sign block hash with key", __func__);

    return true;
}

