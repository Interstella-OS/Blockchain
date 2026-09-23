#include "lib/rsa.h"

void RSAHelper::generate_and_save_keys(const vector<string>& user_names, 
                                       const string& pub_filename, 
                                       const string& priv_filename) {
    AutoSeededRandomPool rng;
    json public_data;
    json private_data;

    for (const string& name : user_names) {
        InvertibleRSAFunction parameters;
        parameters.GenerateRandomWithKeySize(rng, 1024);
        CryptoPP::RSA::PrivateKey privateKey(parameters);
        CryptoPP::RSA::PublicKey publicKey(parameters);
        
        string privateStr, publicStr;
        Base64Encoder privSink(new StringSink(privateStr));
        privateKey.DEREncode(privSink);
        privSink.MessageEnd();
        
        Base64Encoder pubSink(new StringSink(publicStr));
        publicKey.DEREncode(pubSink);
        pubSink.MessageEnd();
        
        public_data[name] = { 
            { "public_key", publicStr } 
        };
        private_data[name] = { 
            {"private_key", privateStr} 
        };
    }

    ofstream publicfile(pub_filename);
    publicfile << public_data.dump(4);
    ofstream privatefile(priv_filename);
    privatefile << private_data.dump(4);
}

string RSAHelper::sign_message(const string& message, const string& private_key_b64) {
    CryptoPP::RSA::PrivateKey privateKey;
    StringSource privSource(private_key_b64, true, new Base64Decoder);
    privateKey.BERDecode(privSource);

    AutoSeededRandomPool rng;
    RSASS<PKCS1v15, SHA256>::Signer signer(privateKey);
    
    string signature;
    StringSource(message, true, 
        new SignerFilter(rng, signer, 
            new HexEncoder(new StringSink(signature)) 
        )
    );
    return signature;
}

bool RSAHelper::verify_signature(const string& message, const string& hex_signature, const string& public_key_b64) {
    try {
        CryptoPP::RSA::PublicKey publicKey;
        StringSource pubSource(public_key_b64, true, new Base64Decoder);
        publicKey.BERDecode(pubSource);

        RSASS<PKCS1v15, SHA256>::Verifier verifier(publicKey);
        
        string binary_signature;
        StringSource(hex_signature, true, new HexDecoder(new StringSink(binary_signature)));

        bool result = false;
        StringSource(message + binary_signature, true,
            new SignatureVerificationFilter(
                verifier,
                new ArraySink((CryptoPP::byte*)&result, sizeof(result)),
                SignatureVerificationFilter::PUT_RESULT | SignatureVerificationFilter::SIGNATURE_AT_END
            )
        );
        return result;
    } 
    catch (...) {
        return false;
    }
}