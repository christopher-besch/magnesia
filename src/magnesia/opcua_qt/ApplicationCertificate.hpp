#pragma once

#include <cstddef>

#include <QList>
#include <QSslCertificate>
#include <QString>

namespace magnesia::opcua_qt {
    /**
     * @class ApplicationCertificate
     * @brief Contains all information associated with a certificate
     */
    class ApplicationCertificate {
      public:
        /**
         * @brief Create a self-signed X.509 v3 certificate.
         *
         * @param subject Elements for the subject,
         *        e.g. {"C=DE", "O=SampleOrganization", "CN=Open62541Server@localhost"}
         * @param subject_alt_name Elements for SubjectAltName,
         *        e.g. {"DNS:localhost", "URI:urn:open62541.server.application"}
         * @param key_size_bits Size of the generated key in bits. If set to 0, the maximum key size is used.
         *        Possible values are: 0, 1024 (deprecated), 2048, 4096
         */
        ApplicationCertificate(const QList<QString>& subject, const QList<QString>& subject_alt_name,
                               std::size_t key_size_bits = s_default_key_size);
        /**
         * @brief Creates a Certificate Object from an existing certificate
         *
         * @param private_key Private key
         * @param certificate Certificate
         */
        ApplicationCertificate(const QSslCertificate& private_key, const QSslCertificate& certificate);
        /**
         * @brief Returns the certificate
         */
        [[nodiscard]] const QSslCertificate& getCertificate() const noexcept;
        /**
         * @brief Returns the private key
         */
        [[nodiscard]] const QSslCertificate& getPrivateKey() const noexcept;

      private:
        static const std::size_t s_default_key_size = 2048;
        QSslCertificate          m_private_key;
        QSslCertificate          m_certificate;
    };
} // namespace magnesia::opcua_qt
