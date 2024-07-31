#include "ApplicationCertificate.hpp"

#include <cstddef>
#include <ranges>
#include <string>
#include <vector>

#include <open62541pp/Crypto.h>
#include <open62541pp/Span.h>
#include <open62541pp/types/Builtin.h>

#include <QByteArray>
#include <QList>
#include <QSsl>
#include <QSslCertificate>
#include <QString>

namespace magnesia::opcua_qt {
    opcua::Span<opcua::String> to_span(const QList<QString>& list) {
        auto view =
            list | std::views::transform([](const QString& input) { return opcua::String(input.toStdString()); });
        auto vector = std::vector<opcua::String>{view.begin(), view.end()};
        return opcua::Span<opcua::String>{vector};
    }

    ApplicationCertificate::ApplicationCertificate(const QList<QString>& subject,
                                                   const QList<QString>& subject_alt_name, std::size_t key_size_bits) {
        const auto result_pair = opcua::crypto::createCertificate(to_span(subject), to_span(subject_alt_name),
                                                                  key_size_bits, opcua::crypto::CertificateFormat::DER);

        m_certificate = QSslCertificate{QByteArray{std::string{result_pair.certificate.get()}.c_str()}, QSsl::Der};
        m_private_key = QSslCertificate{QByteArray{std::string{result_pair.privateKey.get()}.c_str()}, QSsl::Der};
    }

    ApplicationCertificate::ApplicationCertificate(const QSslCertificate& private_key,
                                                   const QSslCertificate& certificate)
        : m_private_key(private_key), m_certificate(certificate) {}

    const QSslCertificate& ApplicationCertificate::getCertificate() const noexcept {
        return m_certificate;
    }

    const QSslCertificate& ApplicationCertificate::getPrivateKey() const noexcept {
        return m_private_key;
    }
} // namespace magnesia::opcua_qt
