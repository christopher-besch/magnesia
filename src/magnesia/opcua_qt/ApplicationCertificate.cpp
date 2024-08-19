#include "ApplicationCertificate.hpp"

#include "../qt_version_check.hpp"

#include <cstddef>
#include <ranges>
#include <utility>
#include <vector>

#include <open62541pp/Crypto.h>
#include <open62541pp/Span.h>
#include <open62541pp/types/Builtin.h>

#include <QByteArray>
#include <QList>
#include <QSsl>
#include <QSslCertificate>
#include <QSslKey>
#include <QString>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtTypes>
#else
#include <QtGlobal>
#endif

namespace {
    opcua::Span<opcua::String> to_span(const QList<QString>& list) {
        auto view =
            list | std::views::transform([](const QString& input) { return opcua::String(input.toStdString()); });
        auto vector = std::vector<opcua::String>{view.begin(), view.end()};
        return opcua::Span<opcua::String>{vector};
    }
} // namespace

namespace magnesia::opcua_qt {

    ApplicationCertificate::ApplicationCertificate(const QList<QString>& subject,
                                                   const QList<QString>& subject_alt_name, std::size_t key_size_bits) {
        const auto result_pair = opcua::crypto::createCertificate(to_span(subject), to_span(subject_alt_name),
                                                                  key_size_bits, opcua::crypto::CertificateFormat::DER);

        auto certificate = result_pair.certificate.get();
        m_certificate    = QSslCertificate{
            QByteArray{certificate.data(), static_cast<qsizetype>(certificate.size())},
            QSsl::Der
        };
        auto private_key = result_pair.privateKey.get();
        m_private_key    = QSslKey{
            QByteArray{private_key.data(), static_cast<qsizetype>(private_key.size())},
            // TODO: use actual key algorithm type, not just always rsa
            QSsl::Rsa, QSsl::Der
        };
    }

    ApplicationCertificate::ApplicationCertificate(QSslKey private_key, const QSslCertificate& certificate)
        : m_private_key(std::move(private_key)), m_certificate(certificate) {}

    const QSslCertificate& ApplicationCertificate::getCertificate() const noexcept {
        return m_certificate;
    }

    const QSslKey& ApplicationCertificate::getPrivateKey() const noexcept {
        return m_private_key;
    }
} // namespace magnesia::opcua_qt
