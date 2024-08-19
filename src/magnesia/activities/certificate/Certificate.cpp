#include "Certificate.hpp"

#include "../../Activity.hpp"
#include "../../Application.hpp"
#include "../../database_types.hpp"
#include "../../opcua_qt/ApplicationCertificate.hpp"

#include <cstddef>
#include <optional>

#include <open62541pp/ErrorHandling.h>

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QLoggingCategory>
#include <QPushButton>
#include <QScrollArea>
#include <QSsl>
#include <QSslCertificate>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtTypes>
#else
#include <QtGlobal>
#endif

namespace {
    Q_LOGGING_CATEGORY(lc_certificate_activity, "magnesia.certificate")
} // namespace

namespace magnesia::activities::certificate {
    /*
     * Activity for creating and displaying certificates.
     *
     * @param cert_id StorageId of the Certificate to be shown or nullopt for a new certificate.
     * @param parent parent widget of the Activity.
     */
    Certificate::Certificate(std::optional<StorageId> cert_id, QWidget* parent)
        : Activity(parent), m_available_subjects{
                                {"Common Name",                    "CN="},
                                {"Organization",                   "O=" },
                                {"Locality",                       "L=" },
                                {"Organizational Unit",            "OU="},
                                {"Country (ISO3166 Country Code)", "C=" },
                                {"State",                          "ST="},
    } {
        auto* layout = new QVBoxLayout;

        auto* scroll_area = new QScrollArea;
        scroll_area->setWidgetResizable(true);
        layout->addWidget(scroll_area);

        if (cert_id.has_value()) {
            displayCertificate(scroll_area, *cert_id);
        } else {
            createNewCertificate(scroll_area);
        }

        setLayout(layout);
    }

    void Certificate::createNewCertificate(QScrollArea* parent) {
        auto* layout = new QVBoxLayout;

        auto* widget = new QWidget;
        widget->setLayout(layout);
        parent->setWidget(widget);

        auto* subject_box = new QGroupBox("Certificate Information");
        layout->addWidget(subject_box);

        auto* subject_form = new QFormLayout;
        subject_box->setLayout(subject_form);

        for (const QString& property_name : m_available_subjects.keys()) {
            const CertificateProperty property{
                .name     = new QLabel{property_name},
                .lineedit = new QLineEdit,
            };
            subject_form->addRow(property.name, property.lineedit);
            m_certificate_properties.append(property);
        }
        auto* certificate_box = new QGroupBox("Certificate Settings");
        auto* cert_form       = new QFormLayout;
        certificate_box->setLayout(cert_form);

        m_cert_size = new QComboBox;
        m_cert_size->addItem("1024");
        m_cert_size->addItem("2048");
        m_cert_size->addItem("4096");
        m_cert_size->setCurrentIndex(1);
        auto* cert_label = new QLabel("Certificate Key Size");
        cert_form->addRow(cert_label, m_cert_size);

        layout->addWidget(certificate_box);

        m_status_label = new QLabel;
        layout->addWidget(m_status_label);

        m_create_button = new QPushButton("Create Certificate");
        layout->addWidget(m_create_button);
        connect(m_create_button, &QPushButton::clicked, this, &Certificate::saveCertificate);
    }

    namespace {
        std::optional<QString> get_cert_info(const QSslCertificate& certificate, QSslCertificate::SubjectInfo subject) {
            auto info_list = certificate.subjectInfo(subject);
            if (info_list.isEmpty()) {
                return std::nullopt;
            }
            return info_list.first();
        }

        QString get_key_algo(QSsl::KeyAlgorithm algorithm) {
            switch (algorithm) {
                case QSsl::KeyAlgorithm::Opaque:
                    return "";
                case QSsl::KeyAlgorithm::Rsa:
                    return "RSA";
                case QSsl::KeyAlgorithm::Dsa:
                    return "DSA";
                case QSsl::KeyAlgorithm::Ec:
                    return "Elliptic Curve";
                case QSsl::KeyAlgorithm::Dh:
                    return "Diffie-Hellman";
            }
            return "";
        }

        void add_cert_info(QFormLayout* layout, const QString& label_text, const QString& info) {
            auto* label = new QLabel(label_text);
            auto* text  = new QLineEdit(info);
            text->setReadOnly(true);
            layout->addRow(label, text);
        }

        void add_cert_info(QFormLayout* layout, const QString& label_text, std::optional<QString> info) {
            if (info.has_value()) {
                add_cert_info(layout, label_text, *info);
            }
        }
    } // namespace

    void Certificate::displayCertificate(QScrollArea* parent, StorageId cert_id) {
        auto* cert_box = new QGroupBox("Certificate Information");
        parent->setWidget(cert_box);
        auto* layout = new QFormLayout;
        cert_box->setLayout(layout);
        auto certificate = Application::instance().getStorageManager().getCertificate(cert_id);
        if (!certificate.has_value()) {
            layout->addWidget(new QLabel("<font color='red'>Certificate not found</font>"));
            qCWarning(lc_certificate_activity) << "Asked to display certificate with unknown id:" << cert_id;
            return;
        }
        qCDebug(lc_certificate_activity) << "Displaying Certificate" << cert_id << *certificate;

        add_cert_info(layout, "Common Name", get_cert_info(*certificate, QSslCertificate::CommonName));
        add_cert_info(layout, "Organization", get_cert_info(*certificate, QSslCertificate::Organization));
        add_cert_info(layout, "Locality", get_cert_info(*certificate, QSslCertificate::LocalityName));
        add_cert_info(layout, "Organizational Unit",
                      get_cert_info(*certificate, QSslCertificate::OrganizationalUnitName));
        add_cert_info(layout, "Country", get_cert_info(*certificate, QSslCertificate::CountryName));
        add_cert_info(layout, "State", get_cert_info(*certificate, QSslCertificate::StateOrProvinceName));
        {
            auto alternative_names = certificate->subjectAlternativeNames();
            if (auto iter = alternative_names.constFind(QSsl::AlternativeNameEntryType::DnsEntry);
                iter != alternative_names.cend()) {
                add_cert_info(layout, "Domain(s)", *iter);
            }
        }
        add_cert_info(layout, "Expiry Data", certificate->expiryDate().toString());
        add_cert_info(layout, "Key size", QString::number(certificate->publicKey().length()));
        add_cert_info(layout, "Key Algorithm", get_key_algo(certificate->publicKey().algorithm()));
    }

    void Certificate::saveCertificate() {
        m_create_button->setEnabled(false);
        m_status_label->setText("");

        QList<QString>       subject;
        const QList<QString> subject_alt_name{"DNS:localhost"};
        bool                 common_name = false;
        for (auto [name, lineedit] : m_certificate_properties) {
            if (lineedit->text().isEmpty()) {
                continue;
            }
            if (auto type = m_available_subjects.constFind(name->text()); type != m_available_subjects.cend()) {
                subject.append(*type + lineedit->text());

                if (*type == "CN=") {
                    common_name = true;
                }
            }
        }
        if (!common_name) {
            m_status_label->setText("<font color='red'>Common Name is required, no certificate was created.</font>");
            m_create_button->setEnabled(true);
            return;
        }

        try {
            auto certificate = opcua_qt::ApplicationCertificate(
                subject, subject_alt_name, static_cast<std::size_t>(m_cert_size->currentText().toInt()));
            auto storage_id = Application::instance().getStorageManager().storeApplicationCertificate(certificate);
            qCInfo(lc_certificate_activity) << "Created certificate with id:" << storage_id;

            Application::instance().getRouter().route("certificate:view?storage-id=" + QString::number(storage_id));
            Application::instance().closeActivity(this);
        } catch (const opcua::CreateCertificateError& ex) {
            m_status_label->setText("<font color='red'>Failed to create certificate</font>");
            qCWarning(lc_certificate_activity) << "Failed to create certificate:" << ex.what();
            m_create_button->setEnabled(true);
            return;
        } catch (const opcua::BadStatus& ex) {
            m_status_label->setText("<font color='red'>Failed to create certificate</font>");
            qCWarning(lc_certificate_activity) << "Failed to create certificate:" << ex.code() << ex.what();
            m_create_button->setEnabled(true);
            return;
        }
    }
} // namespace magnesia::activities::certificate
