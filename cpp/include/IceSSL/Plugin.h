// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_PLUGIN_H
#define ICE_SSL_PLUGIN_H

#include <IceUtil/Time.h>
#include <Ice/Plugin.h>
#include <Ice/ConnectionF.h>

#include <vector>

// For struct sockaddr_in
#ifdef _WIN32
#   include <winsock2.h>
#else
#   include <netinet/in.h>
#endif

#ifndef ICE_SSL_API
#   ifdef ICE_SSL_API_EXPORTS
#       define ICE_SSL_API ICE_DECLSPEC_EXPORT
#    else
#       define ICE_SSL_API ICE_DECLSPEC_IMPORT
#    endif
#endif

//
// SSL_CTX is the OpenSSL type that holds configuration settings for
// all SSL connections.
//
typedef struct ssl_ctx_st SSL_CTX;

//
// X509 is the OpenSSL type that represents a certificate.
//
typedef struct x509_st X509;

//
// EVP_PKEY is the OpenSSL type that represents a public key.
//
typedef struct evp_pkey_st EVP_PKEY;

namespace IceSSL
{

//
// This exception is thrown if the certificate cannot be read.
//
class CertificateReadException : public IceUtil::Exception
{
public:

    CertificateReadException(const char*, int, const std::string&);
    virtual const std::string ice_name() const;
    virtual IceUtil::Exception* ice_clone() const;
    virtual void ice_throw() const;

    std::string reason;

private:

    static const char* _name;
};

//
// This exception is thrown if the certificate cannot be encoded.
//
class CertificateEncodingException : public IceUtil::Exception
{
public:

    CertificateEncodingException(const char*, int, const std::string&);
    virtual const std::string ice_name() const;
    virtual IceUtil::Exception* ice_clone() const;
    virtual void ice_throw() const;

    std::string reason;

private:

    static const char* _name;
};

//
// Forward declaration.
//
class Certificate;
typedef IceUtil::Handle<Certificate> CertificatePtr;

//
// A representation of a PublicKey.
//
class ICE_SSL_API PublicKey : public IceUtil::Shared
{
    ~PublicKey();

    EVP_PKEY* key() const;

private:

    PublicKey(EVP_PKEY*);
    friend class Certificate;

    EVP_PKEY* _key;
};
typedef IceUtil::Handle<PublicKey> PublicKeyPtr;

//
// This convenience class is a wrapper around OpenSSL's X509 type.
// The interface is inspired by java.security.cert.X509Certificate.
//
class ICE_SSL_API Certificate : public IceUtil::Shared
{
public:

    //
    // Construct a certificate using a X509*. The Certificate assumes
    // ownership of the X509* struct.
    //
    Certificate(X509*);
    ~Certificate();

    //
    // Load the certificate from a file. The certificate must use the
    // PEM encoding format. Raises CertificateReadException if the
    // file cannot be read.
    //
    static CertificatePtr load(const std::string&);

    //
    // Decode a certificate from a string that uses the PEM encoding format.
    // Raises CertificateEncodingException if an error occurs.
    //
    static CertificatePtr decode(const std::string&);

    bool operator==(const Certificate&) const;
    bool operator!=(const Certificate&) const;

    //
    // Get the certificate's public key.
    //
    PublicKeyPtr getPublicKey() const;

    //
    // Verify that this certificate was signed by the given public
    // key. Returns true if signed, false otherwise.
    //
    bool verify(const PublicKeyPtr&) const;

    //
    // Return a string encoding of the certificate in PEM format.
    //
    std::string encode() const;

    //
    // Checks that the certificate is currently valid, that is, the current
    // date falls between the validity period given in the certificate.
    //
    bool checkValidity() const;

    //
    // Checks that the certificate is valid at the given time.
    //
    bool checkValidity(const IceUtil::Time&) const;

    //
    // Get the not-after validity time.
    //
    IceUtil::Time getNotAfter() const;

    //
    // Get the not-before validity time.
    //
    IceUtil::Time getNotBefore() const;

    //
    // Get the serial number. This is an arbitrarily large number.
    //
    std::string getSerialNumber() const;

    //
    // Get the signature algorithm name used to sign the certificate.
    //
    //std::string getSigAlgName() const;

    //
    // Get the signature algorithm OID string from the certificate.
    //
    //std::string getSigAlgOID() const;

    //
    // Get the issuer's distinguished name (DN).
    //
    std::string getIssuerDN() const;

    //
    // Get the values in the issuer's alternative names extension.
    //
    // The returned list contains a pair of int, string.
    //
    // otherName                       [0]     OtherName
    // rfc822Name                      [1]     IA5String
    // dNSName                         [2]     IA5String
    // x400Address                     [3]     ORAddress
    // directoryName                   [4]     Name
    // ediPartyName                    [5]     EDIPartyName
    // uniformResourceIdentifier       [6]     IA5String
    // iPAddress                       [7]     OCTET STRING
    // registeredID                    [8]     OBJECT IDENTIFIER
    //
    // rfc822Name, dNSName, directoryName and
    // uniformResourceIdentifier data is returned as a string.
    //
    // iPAddress is returned in dotted quad notation. IPv6 is not
    // currently supported.
    //
    // The remainder of the data will result in an empty string. Use the raw
    // X509* certificate to obtain these values.
    //
    std::vector<std::pair<int, std::string> > getIssuerAlternativeNames();

    //
    // Get the subject's distinguished name (DN).
    //
    std::string getSubjectDN() const;

    //
    // See the comment for getIssuerAlternativeNames.
    //
    std::vector<std::pair<int, std::string> > getSubjectAlternativeNames();

    //
    // Retrieve the certificate version number.
    //
    int getVersion() const;

    //
    // Stringify the certificate. This is a human readable version of
    // the certificate, not a DER or PEM encoding.
    //
    std::string toString() const;

    //
    // Retrieve the X509 value wrapped by this object. The reference count
    // of the X509 value is not incremented, therefore it is only valid
    // for the lifetime of this object unless the caller increments its
    // reference count explicitly using X509_dup.
    //
    X509* getCert() const;

private:

    X509* _cert;
};

//
// ConnectionInfo contains information that may be of use to a
// CertificateVerifier or an application that wants information
// about its peer.
//
struct ConnectionInfo
{
    //
    // The certificate chain. This may be empty if the peer did not
    // supply a certificate. The peer's certificate (if any) is the
    // first one in the chain.
    //
    std::vector<CertificatePtr> certs;

    //
    // The name of the negotiated cipher.
    //
    std::string cipher;

    //
    // The local TCP/IP host & port.
    //
    struct sockaddr_in localAddr;

    //
    // The remote TCP/IP host & port.
    //
    struct sockaddr_in remoteAddr;
};

//
// An application can customize the certificate verification process
// by implementing the CertificateVerifier interface.
//
class CertificateVerifier : public IceUtil::Shared
{
public:

    //
    // Return false if the connection should be rejected, or true to
    // allow it.
    //
    virtual bool verify(const ConnectionInfo&) = 0;
};
typedef IceUtil::Handle<CertificateVerifier> CertificateVerifierPtr;

//
// In order to read an encrypted file, such as one containing a
// private key, OpenSSL requests a password from IceSSL. The password
// can be defined using an IceSSL configuration property, but a
// plain-text password is a security risk. If a password is not
// supplied via configuration, IceSSL allows OpenSSL to prompt the
// user interactively.  This may not be desirable (or even possible),
// so the application can supply an implementation of PasswordPrompt
// to take responsibility for obtaining the password.
//
// Note that the password is needed during plugin initialization, so
// in general you will need to delay initialization (by defining
// IceSSL.DelayInit=1), configure the PasswordPrompt, then manually
// initialize the plugin.
//
class PasswordPrompt : public IceUtil::Shared
{
public:

    //
    // The getPassword method may be invoked repeatedly, such as when
    // several encrypted files are opened, or when multiple password
    // attempts are allowed.
    //
    virtual std::string getPassword() = 0;
};
typedef IceUtil::Handle<PasswordPrompt> PasswordPromptPtr;

class Plugin : public Ice::Plugin
{
public:

    //
    // Initialize the IceSSL plugin. An application may supply its own
    // SSL_CTX objects to configure the SSL contexts for client
    // (outgoing) and server (incoming) connections. If an argument is
    // nonzero, the plugin skips its normal property-based
    // configuration.
    // 
    virtual void initialize(SSL_CTX* context = 0) = 0;

    //
    // Establish the certificate verifier object. This should be done
    // before any connections are established.
    //
    virtual void setCertificateVerifier(const CertificateVerifierPtr&) = 0;

    //
    // Establish the password prompt object. This must be done before
    // the plugin is initialized.
    //
    virtual void setPasswordPrompt(const PasswordPromptPtr&) = 0;

    //
    // Obtain the SSL context. If you need to customize the context,
    // you should do it before any SSL connections are established.
    //
    virtual SSL_CTX* context() = 0;
};
typedef IceUtil::Handle<Plugin> PluginPtr;

//
// Thrown if getConnectionInfo cannot retrieve the ConnectionInfo.
//
class ConnectionInvalidException : public IceUtil::Exception
{
public:

    ConnectionInvalidException(const char*, int, const std::string&);
    virtual const std::string ice_name() const;
    virtual IceUtil::Exception* ice_clone() const;
    virtual void ice_throw() const;

    std::string reason;

private:

    static const char* _name;
};

//
// This function obtains a ConnectionInfo value that describes a
// Connection. The function raises ConnectionInvalidException if the
// connection is closed or is not an SSL connection.
//
ICE_SSL_API ConnectionInfo getConnectionInfo(const ::Ice::ConnectionPtr&);

}

#endif
