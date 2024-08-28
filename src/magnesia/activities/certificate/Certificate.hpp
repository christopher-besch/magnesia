#pragma once

#include "../../Activity.hpp"
#include "../../ActivityMetadata.hpp"
#include "../../Application.hpp"
#include "../../database_types.hpp"
#include "CertificateURLHandler.hpp"

#include <map>
#include <optional>
#include <vector>

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QString>
#include <QWidget>
#include <qtmetamacros.h>

namespace magnesia::activities::certificate {
    /*
     * A widget representing a CertificateProperty.
     */
    struct CertificateProperty {
        QLabel*    name;
        QLineEdit* lineedit;
    };

    /**
     * Class for the Certificate Activity
     */
    class Certificate : public Activity {
        Q_OBJECT

      public:
        /**
         * @brief The constructor creates a new Activity displaying a certificate, if a id is given, or displaying a
         * form to create a new one, if the given id is std::nullopt.
         *
         * @param cert_id id of a certificate to show or std::nullopt if a new one should be created
         * @param parent parent of this QWidget
         */
        explicit Certificate(std::optional<StorageId> cert_id, QWidget* parent = nullptr);

      private:
        void        createNewCertificate(QScrollArea* parent);
        static void displayCertificate(QScrollArea* parent, StorageId cert_id);

      private slots:
        void saveCertificate();

      private:
        std::map<QString, QString>       m_available_subjects;
        std::vector<CertificateProperty> m_certificate_properties;
        QComboBox*                       m_cert_size{nullptr};
        QLabel*                          m_status_label{nullptr};
        QPushButton*                     m_create_button{nullptr};
    };

    inline constexpr ActivityMetadata metadata{
        .name = u"Certificate",
        .global_init =
            [] {
                Application::instance().getRouter().setStaticSchemeHandler("certificate", new CertificateURLHandler);
            },
    };
} // namespace magnesia::activities::certificate
