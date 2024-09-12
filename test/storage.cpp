#include "HistoricServerConnection.hpp"
#include "Layout.hpp"
#include "SQLStorageManager.hpp"
#include "SettingsManager.hpp"
#include "database_types.hpp"
#include "opcua_qt/ApplicationCertificate.hpp"
#include "opcua_qt/abstraction/MessageSecurityMode.hpp"
#include "settings.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <optional>
#include <set>
#include <string>

#include <QCoreApplication>
#include <QDate>
#include <QJsonDocument>
#include <QSsl>
#include <QSslCertificate>
#include <QSslKey>
#include <QUrl>
#include <QVariant>

namespace {
    constexpr auto* cert1_string = "-----BEGIN CERTIFICATE-----\n"
                                   "MIIF2zCCA8OgAwIBAgIUMcQOZZ7Je1v247DlaqMB0fZ1yFEwDQYJKoZIhvcNAQEL\n"
                                   "BQAwfTELMAkGA1UEBhMCREUxHTAbBgNVBAgMFEJhZGVuLVfDg8K8cnR0ZW1iZXJn\n"
                                   "MRIwEAYDVQQHDAlLYXJsc3J1aGUxEzARBgNVBAoMCkZyYXVuaG9mZXIxDTALBgNV\n"
                                   "BAsMBElPU0IxFzAVBgNVBAMMDm1hZ25lc2lhX2NlcnQxMB4XDTI0MDgwMjEyNTIz\n"
                                   "N1oXDTI1MDgwMjEyNTIzN1owfTELMAkGA1UEBhMCREUxHTAbBgNVBAgMFEJhZGVu\n"
                                   "LVfDg8K8cnR0ZW1iZXJnMRIwEAYDVQQHDAlLYXJsc3J1aGUxEzARBgNVBAoMCkZy\n"
                                   "YXVuaG9mZXIxDTALBgNVBAsMBElPU0IxFzAVBgNVBAMMDm1hZ25lc2lhX2NlcnQx\n"
                                   "MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA3MIIyvUc+PkRRN93gSoi\n"
                                   "S6341bXZGG6m11xSe6pl8NslYStzVSTO9iyXOaCLjubg1ZYLxbnAafxcOLueIM0Q\n"
                                   "mYQmfKI6BaoWF+8FKduA8eo1t+hHL/YqmrhvDXT/9FIRTIPRY4f/ACixFXQ1zD/z\n"
                                   "iyI5ODaMZ2CIXVPXhwSIu4Oh5VniH2sBUFR1QsAV7RimwrrEHzVdh+qRNHItVKyO\n"
                                   "hqjkWPDGvk3MH2kTJ2XrsUXSBc0ioYPp+HzT03IcxJ7DB0rHdOvzf2AHGlHP0khB\n"
                                   "gCwYGK2KXE3/NPtFyythjRnb+F84V22DlLzqX9bSHEJ/xy09RHiC3V7rDFRRVO1J\n"
                                   "CIYtx5KFX4mavXgdC+/+lygx4+EGrtTOr5ehC6D/F377laCwp9DQpPCuOBDyDut3\n"
                                   "xJJILnbrJ11OuUYh0Wgz82zR/GTY86PvdPK9nXvteAxps5Q1/baApey+06YmrJS7\n"
                                   "xVJY8Pj5HG15d0SCxNyepcVng8LT/K1p3iIn4Pumk0cW4bULvxwLi2f3dJGAbMsh\n"
                                   "nel4HippqKfslRhsOqYKT3YzMFcAXYq+wQt0ZHzKdWYbvrL0YWMmwyTO+fdkT8XR\n"
                                   "+f9rRAApRl/E6+TWOk68aETEso/Loki5qaBoSHmcRPJUFLKghvoX4Ntq/PZqo6Qf\n"
                                   "mNXI/xDXe+WXu98Qt1PoiD0CAwEAAaNTMFEwHQYDVR0OBBYEFLiiL3c4L1BNaPj6\n"
                                   "6nVwPQhJo/4IMB8GA1UdIwQYMBaAFLiiL3c4L1BNaPj66nVwPQhJo/4IMA8GA1Ud\n"
                                   "EwEB/wQFMAMBAf8wDQYJKoZIhvcNAQELBQADggIBAAOms4b9eIW4s2XOPJK8qOnZ\n"
                                   "Erd0bEt+bRZOE8FJg7IBTYzfrgTMzyEMlB0aBZ6cTKsKS7smV6QIZ9wymFW4PMXr\n"
                                   "IS831+VVSZJeYpx5jHaBIF2CuQ0+BIgj3OG1a+V9v8wW7knP+uAfsUurOig/KXrO\n"
                                   "10sDSChi+o2Cqx8hIC3BDUP5CQ+TmKcaJrGrY5XF643JKw/a/w89pUAgGQWZ7DnH\n"
                                   "uq2UYbE9j8OAdxDOdmEPaDe/z+1+8kPncP8Qbpg+5YxbCVtNvDjeyUkyYwfJ+D35\n"
                                   "ZAid216l2XxNEfVLChtRrWOlR1mIWBzeguMGG+9lJo9NTIFTfcgkuroM79M/g0gl\n"
                                   "mOayKydgowQqe796k9FXzkZRZkbLxVVDQFq53hwU03v6NYcWUXLbdtLIA4XNRC29\n"
                                   "RXn8CyVID+Bj540/objuA3YSlyu+wPEsSJP9BsXAOfkYavlJtuCuwmKWvSi9flth\n"
                                   "+7V4Z5/G0VktAP/agadJXHDKVrhcxK8oltNWosoOZFO2zmR1sfEfb51hIXJhMDsZ\n"
                                   "8znHIzRQtH0eNUSpX7uCaxq4PSQeqb66O2Ruzovzq3O/zTxP9HU+fyYfdrpGslIG\n"
                                   "xUQxQIAZ04A1l6nscTIV5Vt+T5R8ei+jPJ1nQ2V1y5gYIfnbcykLf14XmpoYt5/Q\n"
                                   "Q0v5BG9gzAajcEXdQ3cC\n"
                                   "-----END CERTIFICATE-----\n";
    constexpr auto* cert2_string = "-----BEGIN CERTIFICATE-----\n"
                                   "MIIF2zCCA8OgAwIBAgIUPrI73OU0PgBuuXpoSqTjlsRyegMwDQYJKoZIhvcNAQEL\n"
                                   "BQAwfTELMAkGA1UEBhMCREUxHTAbBgNVBAgMFEJhZGVuLVfDg8K8cnR0ZW1iZXJn\n"
                                   "MRIwEAYDVQQHDAlLYXJsc3J1aGUxEzARBgNVBAoMCkZyYXVuaG9mZXIxDTALBgNV\n"
                                   "BAsMBElPU0IxFzAVBgNVBAMMDm1hZ25lc2lhX2NlcnQyMB4XDTI0MDgwMjEyNTgx\n"
                                   "OFoXDTI1MDgwMjEyNTgxOFowfTELMAkGA1UEBhMCREUxHTAbBgNVBAgMFEJhZGVu\n"
                                   "LVfDg8K8cnR0ZW1iZXJnMRIwEAYDVQQHDAlLYXJsc3J1aGUxEzARBgNVBAoMCkZy\n"
                                   "YXVuaG9mZXIxDTALBgNVBAsMBElPU0IxFzAVBgNVBAMMDm1hZ25lc2lhX2NlcnQy\n"
                                   "MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAuZ/JONJjBUFSAW4tiCnk\n"
                                   "Xg2ymlYtWcTXFozIff+Ils954ktEt3D7ZyK8A91pDeConeTQlVaykUHwpzekjze4\n"
                                   "+2qSScxKKxbft4RMu8w0hvwIWa9VcRWL392IksJTSGxQN3xrj+7xoGiC2eXf+jnV\n"
                                   "QHI3Y3OcGuVE8FCQeGxgF/Zbac6HwDAXF7arQr/PweLrkYcqY5PhDRyZi5fdL0Oy\n"
                                   "haWz7xSMnNEWWhVByUOVUFXQb+0N4pwCjtvGINFj4Mv0752D3wAAhjTKn70jJEiJ\n"
                                   "wkmcgFGk/ERO/o+ujUjSFTHyEUVjc2X5iKYFYCx0ZA/E0sOyaSeDp1aKLNaOxo5O\n"
                                   "t3bvUS9rsAcf+x9Zaol3Ki8xNdkiJyoh+wQqQ+rZ4CdC81Vcv6fPP163kWjTjYaj\n"
                                   "AtO1RxgcbtQoN+Yw4io1viLtco1ORj0ZGEQCTeJbmb+8KOYrur6L5k1zkVFZSmYJ\n"
                                   "tCJDMY4hFs1jxxu4Do9fGEkr1Jz2gwWustTe9W51cwfLry05cgBAtAPwY/HpZa45\n"
                                   "BlVwO56+ibjkQ71otdHtGUou7++g1/DfLmQL+91u6sGqbb29XOta/DfGJeB+Y5Nw\n"
                                   "0E5wtiErxE377NXClgduzPtSeRimIX/oDCTBUbPpD4hfzeJ0Huo86K+OfLiUfucp\n"
                                   "e/FrMxDz3CiDWVwT2aBd6ZsCAwEAAaNTMFEwHQYDVR0OBBYEFDdZy0gM+urcsora\n"
                                   "CYeh6k/Uc1v0MB8GA1UdIwQYMBaAFDdZy0gM+urcsoraCYeh6k/Uc1v0MA8GA1Ud\n"
                                   "EwEB/wQFMAMBAf8wDQYJKoZIhvcNAQELBQADggIBADAIBpUzLcc+La2TqBrdn2PJ\n"
                                   "PbhrGEjU/tRfGbopr7u+zcr82DIMd59ido3QPLyOg5tRQ/KpcXeKb05XoXpRrXbn\n"
                                   "hzTBX2DvqGJ7Utj4w4MpdgaiO7GgqQZT4SEWaHPNWMCSOwe/KC2OAPJzgjVqEaRW\n"
                                   "RjjLIsgvgkfWtqG54IMvbSHKH9MFH3tfkpXns/Q76Ak/J6eZfVr3jc1SwlzMa5pK\n"
                                   "luej212hmcb0ybTvY1mozVDdzFVYJ8CnGPQgK/Mv38TYq9U5encnid6hxUi24NXP\n"
                                   "9TBnkjLrHiQHxJlOnH1XHAWGBg0R5/ssCHLN4G2IUE+MxuFcurEp2FIt4sAztksP\n"
                                   "86KdL66V5SOm75XCLy2mbuAJGdz4pv3iRo/lEA3x+GvcTmf3BLtHTVyBqoXjPIfp\n"
                                   "nmk7cEEfmkoFDkPByy2vrcYELIQ43owiWNisPHfK/llqh3iEGfE7lhDGlG+Vzp7H\n"
                                   "n4o1Y7fhfGt733FTF742L9J7WVDRKtYGISLPuj3jnlu4Sf731BN9MqP9jdAMxBIG\n"
                                   "hvrtLl5UBMuOE8Ac0Bc07iQBnj2Ug6NQgzz+0YqHj2C2otEyrxO0QpDvQC/F+Rpw\n"
                                   "ugV+giQXkjg987LiSldlG/EmeflR6qshepaXF2ZKgvD1zGQ1tpL2g0Mo0TY4THwK\n"
                                   "ySg5VPMX8O1YEju61pBq\n"
                                   "-----END CERTIFICATE-----\n";
    constexpr auto* cert3_string = "-----BEGIN CERTIFICATE-----\n"
                                   "MIIF2zCCA8OgAwIBAgIUIoaPgMuH9I+FmgQAcG+PJIMkJTIwDQYJKoZIhvcNAQEL\n"
                                   "BQAwfTELMAkGA1UEBhMCREUxHTAbBgNVBAgMFEJhZGVuLVfDg8K8cnR0ZW1iZXJn\n"
                                   "MRIwEAYDVQQHDAlLYXJsc3J1aGUxEzARBgNVBAoMCkZyYXVuaG9mZXIxDTALBgNV\n"
                                   "BAsMBElPU0IxFzAVBgNVBAMMDm1hZ25lc2lhX2NlcnQyMB4XDTI0MDgwMjEyNTkw\n"
                                   "NVoXDTI1MDgwMjEyNTkwNVowfTELMAkGA1UEBhMCREUxHTAbBgNVBAgMFEJhZGVu\n"
                                   "LVfDg8K8cnR0ZW1iZXJnMRIwEAYDVQQHDAlLYXJsc3J1aGUxEzARBgNVBAoMCkZy\n"
                                   "YXVuaG9mZXIxDTALBgNVBAsMBElPU0IxFzAVBgNVBAMMDm1hZ25lc2lhX2NlcnQy\n"
                                   "MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAo0Fz7QpjEOcBglzlra7Q\n"
                                   "5Feukqqx1l3T1dQF7lSn9FiyvyOq4et9U7hKced6IWfol+lq2JohByLeMoOya1Iq\n"
                                   "D9s6/71umKtCKAzhphvSfHc6SmdFX1hAuwTbnuCeu7d51Y8wxXlXpXItgVRHTQNO\n"
                                   "D+kAqG6gl2z7Xs7nUeg9D7mBxgALsyK8LyDetP1ceurCgJP8eJAWjoXh3daOyNGx\n"
                                   "c0U3VQnv79ANr2phaj8n7fE6ujcIon8AgI4Ljh4i1QMM+pwMkF9kykwg2QJdJdY9\n"
                                   "4R9He6azhXCG3GtjzIC+rzozZrNZ+dgeZQwGfzYSMU2n+F43nVR3hUvLLeqfO21C\n"
                                   "Naw9hBxn0iyTfpU04QtphaZ5rwwedvBm/b+qjWjA30Z4NWNHmK02P4Li6GB8gpBx\n"
                                   "QiM3KKvAjhfFzi3gngvIEgHWBbsETAprSRY2jbP9+PD/RWPIvRRDEKCHfA+BEV4Q\n"
                                   "cTCxsRRO9YM6xmNjjM5chY1+9KluRlbTX+OjhCF08Ent69BI2hsysTC1Et8ueGlB\n"
                                   "nR1yxdXHqa+Sr8w7h4TEg9H1iO8zbMQuqXYI93HZ/WAAuU/JmVZcr8EE187jibIW\n"
                                   "eT8FaG0LGMdRTdYXNL9TyDALVnWll8KzDCzAQMcJiuPl3Ed0wwUuDwX1wt8UFmXb\n"
                                   "5OkN/FDwIoF+deFAxyZuJjsCAwEAAaNTMFEwHQYDVR0OBBYEFIr+z9FDvJpyDJlo\n"
                                   "3KT6Lxu8eeQOMB8GA1UdIwQYMBaAFIr+z9FDvJpyDJlo3KT6Lxu8eeQOMA8GA1Ud\n"
                                   "EwEB/wQFMAMBAf8wDQYJKoZIhvcNAQELBQADggIBAJ9Pupo/g3ClyxCJeYXe1JLb\n"
                                   "dQJk8+tRJW0zVKiBDMhH4tUWnbZJoQKG8yKvg05ZzYPHoLx18xcpaoq9nDeZzgWh\n"
                                   "ypKj7dqRWLyQdGN5XbkwqCt0GsDys3pgz7Rpt52RppcrNXe4dXnIUGvWimedZyE6\n"
                                   "msHzkpCZG/+RzvKjrKTg6XCg9ebJsqrvcVyj6QtGnu5UP5BUd7ICSO7oQG9JJQ6w\n"
                                   "eatWaxpCuYJb+LeLHTfKsJsBV/o+VVhkd4y4vfRJ13XAugWDqR8hTCt8Gw8y1Vse\n"
                                   "KCSWXT5GmQATnCEGTGCPYlekLBzeKkBPWhHvm/uvHskID8NBGA8RsyeT4Qn+nuRk\n"
                                   "8GyZ4g7cs4+lo6zS/NXq8fWrTvd6Waj7QRwblANkMCJxNHMzo1OFhrDgXKlQLMJI\n"
                                   "mLnwB+YkqkkzBlgJF5J49YTkz9NcPL6oMkugutB6IBKlbna2tttZSsEkntmu+Xoj\n"
                                   "/6lW6Di3t6iXFFbvn6qoAFuVQI84NpemZskU4Nfj/yWakJDOoz/PpR/E+xkKFBJE\n"
                                   "YxDSRMIxXqQMBMETyaIQLnH/R8OMP2sXBDnHiVNhgmUZ3uRtL5k7O6Ee0MxTr1Fi\n"
                                   "2s0iKR8MhlOLyo2o4Gpfh3VNFUxg8bTkuQ2xU7BeGZBt34A+UUDA74kKQ9Jc9PsU\n"
                                   "Kr2R9f1Ni5/Bb82wHXfp\n"
                                   "-----END CERTIFICATE-----\n";
    constexpr auto* pk1_string   = "-----BEGIN PRIVATE KEY-----\n"
                                   "MIIJQgIBADANBgkqhkiG9w0BAQEFAASCCSwwggkoAgEAAoICAQDcwgjK9Rz4+RFE\n"
                                   "33eBKiJLrfjVtdkYbqbXXFJ7qmXw2yVhK3NVJM72LJc5oIuO5uDVlgvFucBp/Fw4\n"
                                   "u54gzRCZhCZ8ojoFqhYX7wUp24Dx6jW36Ecv9iqauG8NdP/0UhFMg9Fjh/8AKLEV\n"
                                   "dDXMP/OLIjk4NoxnYIhdU9eHBIi7g6HlWeIfawFQVHVCwBXtGKbCusQfNV2H6pE0\n"
                                   "ci1UrI6GqORY8Ma+TcwfaRMnZeuxRdIFzSKhg+n4fNPTchzEnsMHSsd06/N/YAca\n"
                                   "Uc/SSEGALBgYrYpcTf80+0XLK2GNGdv4XzhXbYOUvOpf1tIcQn/HLT1EeILdXusM\n"
                                   "VFFU7UkIhi3HkoVfiZq9eB0L7/6XKDHj4Qau1M6vl6ELoP8XfvuVoLCn0NCk8K44\n"
                                   "EPIO63fEkkgudusnXU65RiHRaDPzbNH8ZNjzo+908r2de+14DGmzlDX9toCl7L7T\n"
                                   "piaslLvFUljw+PkcbXl3RILE3J6lxWeDwtP8rWneIifg+6aTRxbhtQu/HAuLZ/d0\n"
                                   "kYBsyyGd6XgeKmmop+yVGGw6pgpPdjMwVwBdir7BC3RkfMp1Zhu+svRhYybDJM75\n"
                                   "92RPxdH5/2tEAClGX8Tr5NY6TrxoRMSyj8uiSLmpoGhIeZxE8lQUsqCG+hfg22r8\n"
                                   "9mqjpB+Y1cj/ENd75Ze73xC3U+iIPQIDAQABAoICAAIt/pu7EVWU/KUUhsUf9yA5\n"
                                   "ABq4wFi+fke56eI+M7ejK5/lZ66zcxauUzJBYfdf02BQf7PB/9gcDYYNi4QROkTQ\n"
                                   "b6KDOzWYm74R661hioj48h2rfbPHx10T4pwm/QBQJMsVBxwnji6VzZp5Cq9GV1+B\n"
                                   "j5U86/nOU8ewyaaT4SIRgeeg5vu+mPqj0YI+oXN1RETqcVV0jQSdSIs/GwWkbDDW\n"
                                   "BJErPsRyF13yXJB4RWbl71Wu1D9017p5cwxPVWDh8hTa7xQPYK4FMvTBJ4nmPD1U\n"
                                   "J3XA6uY6lUE+6jjAEtHA67GCBoKjJRgyyFFOGUYoUu4oGxwsthq88pmtJcjD91iz\n"
                                   "6wYVa93LWZCy1NMmNN/28wsJILZJZX+4ZOdWG6XFNsM46EfoSwEXe+iHXi4un/m/\n"
                                   "n186JMjS0a24BgyLg6VOv9132z3yrtLthCv3EIE9oK+UKP4mTlaRu4Q/sH9zOuEk\n"
                                   "I1FGQE111kXSqa0Pdneuu8jsJVRtKdygW0tpbl4rGKSIKp6stR+3lMrnCyybj92D\n"
                                   "Mrne7uBGFeqNuWd+4maevAcgkHyocjBKt4661vpnKTKiMz16TE5x/X4tymlxVXW1\n"
                                   "5twGc/Vqbd9tvQYe1GMm04kV5rz4yn3jb+4EWZ9jb4f+o/wO2pWBXbgC8Hj3SFcG\n"
                                   "ahsu/YWZc/r4HissBOLRAoIBAQD3gfTKYM3joQAB/w/tVokpnifgtpyZeb2emruu\n"
                                   "JkLu6mS/mn7dK5/xYKNWqdBwvqwOyi89avotePxuusUlrWbfmh1ak8J3DEbwvFhb\n"
                                   "aQIf0Sx5OC4DkQlSLKqea40BaQb5FrGHkPY/Nz11w2ESS/jOT/QIWfka9m/x3O0Q\n"
                                   "CW3C/VbOX/HlnooNQEW4xMhbw9u8i6xqN6kmbEjMtbkBMKsod1EpO5doDkXElOxx\n"
                                   "pfKHUkQzlOkZvxp5gmb6Fzf0dA9NdXX5+T1vrGDM4U+KownlTt1B5JvEApxeOMSG\n"
                                   "zY7KwqjIW3xvkhsFbELg3DysRpB9z0E/srU2fRzdzNdoE+ZtAoIBAQDkVR2b4s96\n"
                                   "P77z8s2tY/kYlgxPiW/rCiBm05Thi4BiGJnh6NXw4JK7t7v7k2zgiHwBrkGd6wHb\n"
                                   "u0gvWvRMvH1td5yvlv+N+IhOQSuU+U/b8Vyrh6D2oAJh5a1ARe5lM6wcN7MaJFC4\n"
                                   "gInySbxrqNSlRvlrc0rzj5HXC6VmLcXSo7ITvrB/3DzY4cjfwgLlI+cCVTlHXf66\n"
                                   "0vX2NZyYyqwhKv9JWd5DDyOE6cauUfHR7CTJeFLaOMfF3PmbYcVVHUfzo3p9ijCL\n"
                                   "XK3uI3VbfLXA8JwQKAYDKdfHYlaBklFhQvvzxdYWFnYJ/xriCGxGKKDrfYsjpfoX\n"
                                   "e8bkEQWHpkcRAoIBAEinh1oZDJczXbM4Uk6TRnRHDuGx+Uo8KGN0fH9kj+VePPDZ\n"
                                   "wz/dXHgYrBn2g091SUCWLUPUMVhD2aYPgWvL+RTOQquiEEWt/u0P/MfojU0Vs/5p\n"
                                   "dWhqnNfYcMVMQPoXAIsCFF79OKvZ5Vq6XbZnQsQdSD9jWfKGQNo4xgYdHnFMuLyg\n"
                                   "AXwsC85T/aN9cGsvlUNFpPkRGcBDrSMafRJSXQvl6pc1/P34opZQORArbIQ4q/iQ\n"
                                   "T9TU+kqM8vma8zv5DbzpLBaTCWvMqCW8RgMmVPNRMTX0i50APzKRa28+zBnNBoq6\n"
                                   "AE8oDnKLEx66x7yyghvMSRUPuo4iL+UUCm9s2AkCggEAeHMeJ3nCkW7HQNE5BtrS\n"
                                   "MNIWtG6AB7FT3mqHA2Qf2Z+4rZ+yHFubjEMke1YN/XSCVjCilLfARzXIopKMhV55\n"
                                   "nCGZGlfBgBc3cHCYmQHbyt7MUABAPVmzaAlNF7bMR762cDxB0uyNWIcWPiX/q5to\n"
                                   "cbe7I5+yTkHSYWrXWcuTq8aGzjutyAVaY97XpJIoDNCfsG9cw/Ulx1A6SCOH9GIV\n"
                                   "43lXKk6qHR4DUr4v/Q1lT6QO0+kRa+TAHfSwB/PPsVK8/h2jJE6dxSai6hjyc8yZ\n"
                                   "5KoI6czzPv/3wEpz7+5L188KMlKO2SAILMXSXKKs22XbXd7gY3HPOEwnY3BHRoPE\n"
                                   "wQKCAQEAjASE8lIXIunuuii1DnI2G82z6j6hnM4AKemBh7bvlJkKp8BS8Al6ujEy\n"
                                   "+291He+ZbthcGiDrj7niVZn9GXdYJNTPUlQoxc51dbisxGRt0nJaWI6Bv72crjgt\n"
                                   "7/dLcJS8Y0tvcclBYOSbyUVzKM46xMwoLtIg67q2AkVNnMq+XK2s+0N+EyWwRYga\n"
                                   "LFKIwwmCeuu5a1iDycDu/XdCg+Q4OclJTZBzY/DHfhm6YSynZbaeup071Smgp4ju\n"
                                   "VUe4o2fgHRclSXo1D4abdlecxR6ToVW5hDd6H8mA99jhDQt6Yg386wvUfJRCPc84\n"
                                   "RZLcFXLoYaYmcuFYfVIgPPatw/8xXw==\n"
                                   "-----END PRIVATE KEY-----\n";
    constexpr auto* pk2_string   = "-----BEGIN PRIVATE KEY-----\n"
                                   "MIIJQQIBADANBgkqhkiG9w0BAQEFAASCCSswggknAgEAAoICAQC5n8k40mMFQVIB\n"
                                   "bi2IKeReDbKaVi1ZxNcWjMh9/4iWz3niS0S3cPtnIrwD3WkN4Kid5NCVVrKRQfCn\n"
                                   "N6SPN7j7apJJzEorFt+3hEy7zDSG/AhZr1VxFYvf3YiSwlNIbFA3fGuP7vGgaILZ\n"
                                   "5d/6OdVAcjdjc5wa5UTwUJB4bGAX9ltpzofAMBcXtqtCv8/B4uuRhypjk+ENHJmL\n"
                                   "l90vQ7KFpbPvFIyc0RZaFUHJQ5VQVdBv7Q3inAKO28Yg0WPgy/TvnYPfAACGNMqf\n"
                                   "vSMkSInCSZyAUaT8RE7+j66NSNIVMfIRRWNzZfmIpgVgLHRkD8TSw7JpJ4OnVoos\n"
                                   "1o7Gjk63du9RL2uwBx/7H1lqiXcqLzE12SInKiH7BCpD6tngJ0LzVVy/p88/XreR\n"
                                   "aNONhqMC07VHGBxu1Cg35jDiKjW+Iu1yjU5GPRkYRAJN4luZv7wo5iu6vovmTXOR\n"
                                   "UVlKZgm0IkMxjiEWzWPHG7gOj18YSSvUnPaDBa6y1N71bnVzB8uvLTlyAEC0A/Bj\n"
                                   "8ellrjkGVXA7nr6JuORDvWi10e0ZSi7v76DX8N8uZAv73W7qwaptvb1c61r8N8Yl\n"
                                   "4H5jk3DQTnC2ISvETfvs1cKWB27M+1J5GKYhf+gMJMFRs+kPiF/N4nQe6jzor458\n"
                                   "uJR+5yl78WszEPPcKINZXBPZoF3pmwIDAQABAoICABU5xkKvHA+PxpbPSHmXgLuY\n"
                                   "iWHIlOr87TNw6cQuoinMk71iqfrn+ndyq1ktykOB+wYDEi7heQ3PInuciz6tOoPV\n"
                                   "TeEOcTS4KuDLShL3aNI+o53NxNFni1UP+fv+KJPRfuK5YXtV8V9MVBoihRgR4Oa7\n"
                                   "qMXAhIs2yElKjRqYGezjiNRyf2dkmF9nzfGWCWW8Uv+TGy/Dyyo6qPwjY3FyJknO\n"
                                   "BuNB0EpmRk4KvDkNd1d2WJ1T2BCfklNPojpCd09PQzTbKCcp0uWFx924PPp7KPWE\n"
                                   "gVbS/zCxhu7o8ybIuu0dhvaenp6lfIE5qseyjaSbgQAwzIflstJBvQ8fgbgGsc1q\n"
                                   "w0Wff/KriKwtEordhZBuEpip9GSQ2vR3Xp1NWi5FskAnvRL0q+IzclImGAZ4r4O/\n"
                                   "ANpM8Oi/vDHp28v8FO6m7Sidx7ipjMqLmqPjUXhdG0RTSh6nO908s1BEE9qYgOz2\n"
                                   "WD0IBMfWRJKTnZei1lv92mv106DhIzRbjiRk2oskWxp3M/l4uL0UcBtiKxH8A2kp\n"
                                   "OycfEI/ctuMQ7xHHgmBO5K2suy5/432rqdknEvcRe7rN19F09bMH2nyqGJBSKiMT\n"
                                   "D2CEoq/stOLt2uHymo8YVMbU1zfoQvFAP1X71l3OoJiCsPPJAasQIOfkhy6NOUan\n"
                                   "WsKg7HCSJ5omKpJ8mOQBAoIBAQDvcrATcDPVw7CekWwViey9ybrGkyjXZ530HsEs\n"
                                   "KPpugAgvWKoD9Kmb9r1Y48NUxoAg7hs1BIo+fDdXPk9ugI9kGk/4zg+fw6p80QDk\n"
                                   "ixhBY/te5aPNvBk5YBl4OGIXx3oGEaExCu7rxshFRa4bnqxbeNRJyQTxZKI4Abhq\n"
                                   "QPhEIQi8552MUb2mKjSuHz2zlhpltHfBBmxn/i/eqf4EUGCESYkQAYtT5DE8mtrc\n"
                                   "TejNWVPzIX86bbxxQASgXxZE7WX26z2t4k009OQ56DUlhu+YnBBnHnRvrW5PET9B\n"
                                   "MMWvyx7w1Gd2FRheYzp1vI6hetVe0ctfs/SEK+a9OgqKsH8BAoIBAQDGdJ9f+gtp\n"
                                   "eYre1cwHje70RddskenaObzWjMBwr4szPmFYQio2pc+m3D0x0W+Tj7PfPBfOwrTx\n"
                                   "U6y57nqwMszqAqJPEyhKwHwuLj5fQzprbptNQPer7RhH6cJW1e+alwgxMhBAmlzL\n"
                                   "ZRRruR1qDNhHZBi1ByPecwGxV/H3MSHHe/VoYRxfFphz3qZW53X3CToS+f5o+AgX\n"
                                   "wat7MjmYCv+lrtyt/eHb1hxE56N3dRs3yn932bh/bDDCk+nA7zCEvq0NCSPePKuF\n"
                                   "W33BKbTMb34u/2edCCmzAnkbA42F5/0n0dxD5q+qoFSGmOnw47ilEI8vaqyAtHpx\n"
                                   "tacH54TqhQSbAoIBADWRzP6sTYx3eV4gXrP65XIYXabraWm3jsj80G7WxCBQmIQv\n"
                                   "K+oypHd2Hipy2lJvDl9PpdqYp20PUTo9gHvSW0Ij+NxLDYfKuKuv/qkizUxj1+sF\n"
                                   "KT5v5JzHeStCQf/b6trMuulD7uvkzw1gZ3FacGkOgT/4rxhqUTiCegWbf2+OjGvm\n"
                                   "zYKFDw7LxnlmF+HDpSg31crjawFhBsij82Z3rQAri3R/Q/KCzNkoM+qxq3G1mGh+\n"
                                   "h/5EarJThjVy0IKfMf6p5mSuimNcnmquHqXEqHnWAH1xfM83V8RssWAeTzvjD4sF\n"
                                   "mhf65DbJsFyC0M4jsvTBruQ8WGnxgG+5On6IugECggEAChiIuxM3XWhHfkqWFJRN\n"
                                   "4bNj4CwUH3vsdWW3rIPhqvuzbBABocIRrEVr/4rOCE25MtQVyfjxx40gLl1JdIlB\n"
                                   "D26O30L0O8lLB/ZmSeZbpHd+AxIZB3AKWi40KsK7a7vfONF5LjAsJFTCEzDZpDl/\n"
                                   "DzKyZxMz6e8j2OfGcXLYaxONSoAWN2go3UwoXidsQuL/R1Pcc26aysqZdKz7bRay\n"
                                   "JrWcmGVgBhUAXykBkeQtPEta7LfwwNq6KEvqClygtUAWAaHDZ1zixOgAn8EFZ5kM\n"
                                   "bEul88EkjezESHGH05YXtNziPboQmbeKVUaWMCEFlbAf+KOOnwWPbRV2p9pJUrzt\n"
                                   "nQKCAQBMf6+OjvXpIwkk4rdAXo37QChYpUKrlwIMN4TE4pQH6yULgnO9g5g8oOtH\n"
                                   "RYOo11z2isFCwm+BjM+AQAD2ahXQyT/8zNogyb4Zxnx0MjA/bbXm6hWlqfcF0aM2\n"
                                   "QfvtrHkjREUWeRySAUx1LtK5jo4jtyeBPf0vM78VHSMpAS8/wAGCnZ6PvmJH7/aS\n"
                                   "J1DCeoS1L1l64MzNwMWanvyOrmkRUyQ5r6pX0ZXbFOlbYqqo/gLjgZQvsTofTLcv\n"
                                   "HN4cmylQ3Y96sVqM8vcKvo7SevV8PntEVM94U1Fv1QMpEkaYdvuhp18MwJkGpAEE\n"
                                   "X+IYpw8/RjqfURk9cXM2aA4fSdtY\n"
                                   "-----END PRIVATE KEY-----\n";
    constexpr auto* pk3_string   = "-----BEGIN PRIVATE KEY-----\n"
                                   "MIIJQgIBADANBgkqhkiG9w0BAQEFAASCCSwwggkoAgEAAoICAQCjQXPtCmMQ5wGC\n"
                                   "XOWtrtDkV66SqrHWXdPV1AXuVKf0WLK/I6rh631TuEpx53ohZ+iX6WrYmiEHIt4y\n"
                                   "g7JrUioP2zr/vW6Yq0IoDOGmG9J8dzpKZ0VfWEC7BNue4J67t3nVjzDFeVelci2B\n"
                                   "VEdNA04P6QCobqCXbPtezudR6D0PuYHGAAuzIrwvIN60/Vx66sKAk/x4kBaOheHd\n"
                                   "1o7I0bFzRTdVCe/v0A2vamFqPyft8Tq6NwiifwCAjguOHiLVAwz6nAyQX2TKTCDZ\n"
                                   "Al0l1j3hH0d7prOFcIbca2PMgL6vOjNms1n52B5lDAZ/NhIxTaf4XjedVHeFS8st\n"
                                   "6p87bUI1rD2EHGfSLJN+lTThC2mFpnmvDB528Gb9v6qNaMDfRng1Y0eYrTY/guLo\n"
                                   "YHyCkHFCIzcoq8COF8XOLeCeC8gSAdYFuwRMCmtJFjaNs/348P9FY8i9FEMQoId8\n"
                                   "D4ERXhBxMLGxFE71gzrGY2OMzlyFjX70qW5GVtNf46OEIXTwSe3r0EjaGzKxMLUS\n"
                                   "3y54aUGdHXLF1cepr5KvzDuHhMSD0fWI7zNsxC6pdgj3cdn9YAC5T8mZVlyvwQTX\n"
                                   "zuOJshZ5PwVobQsYx1FN1hc0v1PIMAtWdaWXwrMMLMBAxwmK4+XcR3TDBS4PBfXC\n"
                                   "3xQWZdvk6Q38UPAigX514UDHJm4mOwIDAQABAoICAAFUG7ODF8xLAzWhtQAN6/W6\n"
                                   "3ztkWIlKzRUqVBITlP8NXB/neDH79JQnHernA+L4XfSgLvYt3+VleIyuOYPn8kfN\n"
                                   "zVl9UObDFVc3larz3WyuY6i+ZUGUm+AyVFnLmtFtscYD4d6KgeAmDpwvB/GoVv71\n"
                                   "ikz28D0pRxcKadWfFa5FoJ/qLWyiyMoRWGYJ54AjpdeioukupmnnXE7pIaeHbEFZ\n"
                                   "Bg8LJAM5Sd0sX//2L+3pxI8Ue8rPa/ImHlyQbyuFsvFs9go87dz850th4Lmqmdmb\n"
                                   "KcQKBZTt3c4EygzFDjom5nPRo8EEIP7/cZRJXnIqiW7sOlZXnSdJwHrjjoAy5tLv\n"
                                   "hcMn8eXU1MttoRNCJEcP69yJh1ieyHg79uUdgsiVk0nGfQrstrclETnXqwkStWV8\n"
                                   "EczfN1QZ+FmLGq06JYo9o4KhWNfxBNb1VsgNJZp/MAGPoWNXanAHTZ05r4pwp2wz\n"
                                   "r04lk/E1z/J438gpdKSBw3738x0cqx8OGIyuY9xft6OTycmRiAXgSg4Z677PoTuf\n"
                                   "GzQBp6m53UgDsqRWYq7T/AQMDUHd5MH0Ii7q28PIeML2tzt6VGhcnjsXcf/VO5g2\n"
                                   "Nz7wjIkEigm3R6D4B2B68WtVr9YR08tiOH68XsIgZAb2QAmk0DPlEEdRWDBLnlpo\n"
                                   "O5aFGhgkgisFCoM4/iWVAoIBAQDlKvGbcvQgHypDEtcPo53UngvjBxrjgo2YHfiK\n"
                                   "FEItnRtENU+VOMwXQGG1RdtAlaISAIoQU8wl1lasZWWwk98TIB8Z4Mp2qmSxNxGM\n"
                                   "39RXryd0aXTXZAV6Hg2CnOwqQCEd/7OhThFcy4WPMys4Z5bH8NZtamFWM1jkdTOT\n"
                                   "xa0JlCKpcpr85wrbH2IXfwbz4Va8pu7PRz2ATCn1nlaB1u2LMP7o0sWfa2Ubunuy\n"
                                   "WL/G0fJDNeGuB2hbrRX1JWR+cd/6p2ekSwBhkW7PwqASMOBISazp/giY8fC6CjBx\n"
                                   "Gs/R7AIa17LroMxi1M8kRVnSodZRuCocduLF1DCT4CNscnONAoIBAQC2Xt2eVXYb\n"
                                   "EbRerQBX+xyK0CYbnosyzHn3uI0oqrTd0W7ihTBHm2kj6bccuoPwHh/ONzEqDbj0\n"
                                   "7rkH41DH4OJyMWho/yga3ktl+L9PxgLIZ5OKaJXjPXMVg5KPE18FA86GnNtoUZey\n"
                                   "F8GLPbpWOkgt6GHIcnD/sZAn8VaYHQEPp0gwR4yjXWVQTvXKBjrnQx6ybCjkpnnj\n"
                                   "AombPy5+LgKH1Jqp76UxMGIT0iBUDHwmE9WrvR8TDGPrmZA84l594RmT3P6qA0+F\n"
                                   "a6qHRLKH2KE28OErTW94AGnK6hnHK11wXofTbTyB3kNzq+Zb/HgG8RWHC/XxwD+L\n"
                                   "Eh0Uoj6E7ernAoIBAA4SLPOWg1bVkwq7RWk4aqB7c89kyEnf4kHM1U3lIIsccmgl\n"
                                   "vdUzv5RN57HxsByvE1x3nAKUopxQJNJBB5Vn/vJ1rnMwUj4nCDe5YeJ8e9VGc+25\n"
                                   "6+qtwYhWAA5dBEMz3NSytpzY5kgEVxcL1UOJWMC+EOx287MPRA4DBD+2i4FjDx3I\n"
                                   "Y6vTo98Adk1rB0pCM8kxlsgzX5tZEdp69mTShZNeVsiE6I15UqP5N08kE+JpUa6+\n"
                                   "yDSfT9ERN5/tw9XfuBjz3nFACa3z+BHZqWp0hdDqdSuI5Y4+jMFfjsnjE4+YJijv\n"
                                   "xZSiMup+lplriLBeL4KlBDDrO9FZzgHGaWZnQKECggEAPw2vrCW6LwPBDBbejozI\n"
                                   "wonTEyuOzyJ5bRfG3m+fFy5pGzKXjbqC8KLG58zo/yU7M+5LNlIGR9Ed8kc6z9DG\n"
                                   "GltQ0TSfJiMDiJ8nbip1HdsJ3QnqAOZnGuIh0wwMh0xYpJwpE/lZ7H5gMYdHHLcL\n"
                                   "66tz230N6qn2wIiluvQUQNhCkka6rmUFOW4DdzUOOpUvUNsqjFuuurlO4HlI1uGL\n"
                                   "iHEOhIDR6Z7j2hyFhunGJtgmwTkjPE/FYFj40MQnjXg9gq0e86MvZF7kV959p5BT\n"
                                   "tQ41m+YrCx+BdGHaRc2NgeN9LLGRAQna4mxluhohWa0TF10gTNR+Y9QAb/jkistD\n"
                                   "swKCAQEA0C2o/nKGKIzw8kpI57ouxwzJHmYMNeGGNcafPMY+cXmHpwBvDtbsBtCG\n"
                                   "tjkcl2FvpKY4J2VVlYeGilTFGG3K4ogpmhqHJlpGH2XXR/JSf7fAiH7LtGIs0IQI\n"
                                   "rQI19Q08MyTYRaDwAtit6ktpeR7JwNRuooD1o/LpOc2fn/ct4Rpy9bGdTNC76+XS\n"
                                   "Ucxt8eWboMzIUP5WOJQzGWSy+RHXJ3WrqA4+qB1XxcZcUfpR1WDeg9+G40PZv1k8\n"
                                   "8hmVCf0rmWhUStH/ZdYT+5+S630iUDP1Cyrtiavz/fYbg5rUfE8X1UJ06OgItUKA\n"
                                   "rSQH8b8nlw/++Mk28WGSrLkTLvg0cw==\n"
                                   "-----END PRIVATE KEY-----\n";
} // namespace

class StorageTest : public testing::Test {
  protected:
    int                    argc{1};
    std::string            argv_data{"StorageTest"};
    char*                  argv{argv_data.data()};
    const QCoreApplication qapp{argc, &argv};

    magnesia::SQLStorageManager database{":memory:"};
    magnesia::SettingsManager   settings{&database};
};

namespace magnesia {
    // bugprone-unchecked-optional-access also warns when using optional::value even though it has well defined behavior
    // throwing an exception. We explicitly want this here.
    // NOLINTBEGIN(bugprone-unchecked-optional-access,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    TEST_F(StorageTest, certificate_setting) {
        const auto cert1    = QSslCertificate::fromData(cert1_string, QSsl::EncodingFormat::Pem)[0];
        const auto cert_id1 = database.storeCertificate(cert1);
        const auto cert2    = QSslCertificate::fromData(cert2_string, QSsl::EncodingFormat::Pem)[0];
        database.storeCertificate(cert2);
        const auto cert3 = QSslCertificate::fromData(cert3_string, QSsl::EncodingFormat::Pem)[0];
        database.storeCertificate(cert3);

        const auto pk1    = QSslKey{QByteArray{pk1_string}, QSsl::Rsa, QSsl::EncodingFormat::Pem};
        const auto pk_id1 = database.storeKey(pk1);
        const auto pk2    = QSslKey{QByteArray{pk2_string}, QSsl::Rsa, QSsl::EncodingFormat::Pem};
        const auto pk_id2 = database.storeKey(pk2);
        const auto pk3    = QSslKey{QByteArray{pk3_string}, QSsl::Rsa, QSsl::EncodingFormat::Pem};

        const magnesia::opcua_qt::ApplicationCertificate app_cert1{pk1, cert2};
        const magnesia::opcua_qt::ApplicationCertificate app_cert2{pk3, cert1};
        const magnesia::opcua_qt::ApplicationCertificate app_cert3{pk2, cert3};

        const auto app_cert_id1 = database.storeApplicationCertificate(app_cert1);
        EXPECT_EQ(1, cert_id1);
        const auto app_cert_id2 = database.storeApplicationCertificate(app_cert2);
        EXPECT_EQ(2, app_cert_id2);

        const auto new_cert = database.getCertificate(cert_id1).value();

        EXPECT_EQ(cert1, new_cert);

        const auto new_pk = database.getKey(pk_id1).value();
        EXPECT_EQ(pk1, new_pk);

        const auto new_app_cert = database.getApplicationCertificate(app_cert_id1).value();
        EXPECT_EQ(cert2, new_app_cert.getCertificate());

        const auto certs = database.getAllCertificates();
        EXPECT_EQ(5, certs.size());

        const auto pks = database.getAllKeys();
        EXPECT_EQ(4, pks.size());
        const auto app_certs = database.getAllApplicationCertificates();
        EXPECT_EQ(2, app_certs.size());

        EXPECT_EQ(5, database.getAllCertificates().size());
        EXPECT_EQ(4, database.getAllKeys().size());
        EXPECT_EQ(2, database.getAllApplicationCertificates().size());

        const auto* const domain                  = "my_domain";
        const auto* const certificate_setting     = "my_certificate_setting";
        const auto* const key_setting             = "my_key_setting";
        const auto* const app_certificate_setting = "my_app_certificate_setting";
        settings.defineSettingDomain(
            domain, {
                        std::make_shared<magnesia::CertificateSetting>(certificate_setting, "My Certificate Setting",
                                                                       "This is a Certificate setting"),
                        std::make_shared<magnesia::KeySetting>(key_setting, "My Key Setting", "This is a Key setting"),
                        std::make_shared<magnesia::ApplicationCertificateSetting>(
                            app_certificate_setting, "My Application Certificate Setting",
                            "This is an Application Certificate setting"),
                    });

        EXPECT_FALSE(settings.getCertificateSetting({.name = certificate_setting, .domain = domain}).has_value());
        settings.setCertificateSetting({.name = certificate_setting, .domain = domain}, cert_id1);
        EXPECT_EQ(cert1, settings.getCertificateSetting({.name = certificate_setting, .domain = domain}).value());
        settings.resetSetting({.name = certificate_setting, .domain = domain});
        EXPECT_FALSE(settings.getCertificateSetting({.name = certificate_setting, .domain = domain}).has_value());

        EXPECT_FALSE(settings.getKeySetting({.name = key_setting, .domain = domain}).has_value());
        settings.setKeySetting({.name = key_setting, .domain = domain}, pk_id1);
        EXPECT_EQ(pk1, settings.getKeySetting({.name = key_setting, .domain = domain}).value());
        settings.resetSetting({.name = key_setting, .domain = domain});
        EXPECT_FALSE(settings.getKeySetting({.name = key_setting, .domain = domain}).has_value());

        EXPECT_FALSE(
            settings.getApplicationCertificateSetting({.name = app_certificate_setting, .domain = domain}).has_value());
        settings.setApplicationCertificateSetting({.name = app_certificate_setting, .domain = domain}, app_cert_id1);
        EXPECT_EQ(app_cert1.getCertificate(),
                  settings.getApplicationCertificateSetting({.name = app_certificate_setting, .domain = domain})
                      .value()
                      .getCertificate());
        settings.resetSetting({.name = app_certificate_setting, .domain = domain});
        EXPECT_FALSE(
            settings.getApplicationCertificateSetting({.name = app_certificate_setting, .domain = domain}).has_value());

        database.deleteCertificate(cert_id1);
        database.deleteCertificate(cert_id1);
        database.deleteCertificate(cert_id1);
        database.deleteKey(pk_id1);
        database.deleteKey(pk_id2);
        database.deleteKey(pk_id2);
        database.deleteKey(pk_id2);
        database.deleteCertificate(app_cert_id1);
        database.deleteCertificate(app_cert_id1);
        database.deleteCertificate(app_cert_id1);
    }

    TEST_F(StorageTest, layout_setting) {
        const auto* const domain         = "my_domain";
        const auto* const group_name     = "my_group_name";
        const auto* const first_layout   = R"({"something": "420"})";
        const auto* const second_layout  = R"({"something_else": "69"})";
        const auto* const layout_setting = "my_layout_setting";

        const magnesia::Layout layout{.name = "some name", .json_data = QJsonDocument::fromJson(first_layout)};
        const auto             layout_id = database.storeLayout(layout, group_name, domain);
        const magnesia::Layout layout2{.name = "some name", .json_data = QJsonDocument::fromJson(second_layout)};
        database.storeLayout(layout2, group_name, domain);

        EXPECT_EQ(1, layout_id);

        const auto layouts = database.getAllLayouts(group_name, domain);
        EXPECT_EQ(2, layouts.size());
        settings.defineSettingDomain(
            domain, {
                        std::make_shared<magnesia::LayoutSetting>(layout_setting, "My Layout Setting",
                                                                  "This is a Layout setting", group_name),
                    });

        EXPECT_FALSE(settings.getLayoutSetting({.name = layout_setting, .domain = domain}).has_value());
        settings.setLayoutSetting({.name = layout_setting, .domain = domain}, layout_id);
        EXPECT_EQ(QJsonDocument::fromJson(first_layout),
                  settings.getLayoutSetting({layout_setting, domain}).value().json_data);
        database.deleteLayout(layout_id, group_name, domain);
    }

    TEST_F(StorageTest, historic_server_connection_setting) {
        const auto cert1    = QSslCertificate::fromData(cert1_string, QSsl::EncodingFormat::Pem)[0];
        const auto cert_id1 = database.storeCertificate(cert1);
        const auto cert2    = QSslCertificate::fromData(cert2_string, QSsl::EncodingFormat::Pem)[0];
        const auto cert_id2 = database.storeCertificate(cert2);
        const auto cert3    = QSslCertificate::fromData(cert3_string, QSsl::EncodingFormat::Pem)[0];
        const auto cert_id3 = database.storeCertificate(cert3);

        const auto pk1    = QSslKey{QByteArray{pk1_string}, QSsl::Rsa, QSsl::EncodingFormat::Pem};
        const auto pk_id1 = database.storeKey(pk1);
        const auto pk2    = QSslKey{QByteArray{pk2_string}, QSsl::Rsa, QSsl::EncodingFormat::Pem};
        const auto pk_id2 = database.storeKey(pk2);
        const auto pk3    = QSslKey{QByteArray{pk3_string}, QSsl::Rsa, QSsl::EncodingFormat::Pem};

        const magnesia::opcua_qt::ApplicationCertificate app_cert1{pk1, cert2};
        const magnesia::opcua_qt::ApplicationCertificate app_cert2{pk3, cert1};
        const magnesia::opcua_qt::ApplicationCertificate app_cert3{pk2, cert3};

        const auto app_cert_id1 = database.storeApplicationCertificate(app_cert1);
        const auto app_cert_id2 = database.storeApplicationCertificate(app_cert2);

        const auto* const      domain        = "my_domain";
        const auto* const      group_name    = "my_group_name";
        const auto* const      first_layout  = R"({"something": "420"})";
        const auto* const      second_layout = R"({"something_else": "69"})";
        const magnesia::Layout layout{.name = "some name", .json_data = QJsonDocument::fromJson(first_layout)};
        const auto             layout_id = database.storeLayout(layout, group_name, domain);
        const magnesia::Layout layout2{.name = "some name", .json_data = QJsonDocument::fromJson(second_layout)};
        const auto             layout_id2 = database.storeLayout(layout2, group_name, domain);
        const magnesia::HistoricServerConnection historic_server_connection{
            .server_url                     = QUrl{"https://chris-besch.com"},
            .endpoint_url                   = QUrl{"https://chris-besch.com/404"},
            .endpoint_security_policy_uri   = "I don't even know",
            .endpoint_message_security_mode = magnesia::opcua_qt::MessageSecurityMode::SIGN,
            .username                       = "chris",
            .password                       = "I like cheese!",
            .application_certificate_id     = app_cert_id1,
            .trust_list_certificate_ids     = {cert_id2, cert_id3},
            .revoked_list_certificate_ids   = {cert_id1},
            .last_layout_id                 = layout_id,
            .last_layout_group              = group_name,
            .last_layout_domain             = domain,
            .last_used                      = QDateTime{QDate{2012, 7, 6}, QTime{8, 30, 0}},
        };
        const auto server_con_id = database.storeHistoricServerConnection(historic_server_connection);
        const magnesia::HistoricServerConnection historic_server_connection2{
            .server_url                     = QUrl{"https://chris-besch.com"},
            .endpoint_url                   = QUrl{"https://chris-besch.com/404"},
            .endpoint_security_policy_uri   = "I don't even know",
            .endpoint_message_security_mode = magnesia::opcua_qt::MessageSecurityMode::SIGN,
            .username                       = "chris",
            .password                       = "I like cheese!",
            .application_certificate_id     = app_cert_id2,
            .trust_list_certificate_ids     = {cert_id2, cert_id3},
            .revoked_list_certificate_ids   = {cert_id1},
            .last_layout_id                 = layout_id2,
            .last_layout_group              = group_name,
            .last_layout_domain             = domain,
            .last_used                      = QDateTime{QDate{2012, 7, 6}, QTime{8, 30, 52}}
        };
        const auto server_con_id2 = database.storeHistoricServerConnection(historic_server_connection2);
        EXPECT_EQ(1, server_con_id);

        const auto new_historic_server_con = database.getHistoricServerConnection(server_con_id).value();
        EXPECT_EQ("https://chris-besch.com/404", new_historic_server_con.endpoint_url.toString());

        const auto new_layout = database.getLayout(layout_id, group_name, domain).value();

        const auto server_cons = database.getAllHistoricServerConnections();
        EXPECT_EQ(2, server_cons.size());
        EXPECT_EQ(2, database.getAllHistoricServerConnections().size());

        const auto* const server_connection_setting = "my_historic_server_connection_setting";
        settings.defineSettingDomain(domain,
                                     {
                                         std::make_shared<magnesia::HistoricServerConnectionSetting>(
                                             server_connection_setting, "My HistoricServerConnectionSetting Setting",
                                             "This is a HistoricServerConnection setting"),
                                     });

        EXPECT_FALSE(settings.getHistoricServerConnectionSetting({.name = server_connection_setting, .domain = domain})
                         .has_value());
        settings.setHistoricServerConnectionSetting({.name = server_connection_setting, .domain = domain},
                                                    server_con_id);
        EXPECT_EQ("https://chris-besch.com/404",
                  settings.getHistoricServerConnectionSetting({.name = server_connection_setting, .domain = domain})
                      .value()
                      .endpoint_url.toString());
        settings.resetSetting({.name = server_connection_setting, .domain = domain});
        EXPECT_FALSE(settings.getHistoricServerConnectionSetting({.name = server_connection_setting, .domain = domain})
                         .has_value());

        // the history needs to be deleted first
        database.deleteHistoricServerConnection(server_con_id);
        database.deleteHistoricServerConnection(server_con_id2);
        database.deleteHistoricServerConnection(server_con_id2);
        database.deleteHistoricServerConnection(server_con_id2);
        database.deleteCertificate(cert_id1);
        database.deleteCertificate(cert_id1);
        database.deleteCertificate(cert_id1);
        database.deleteKey(pk_id1);
        database.deleteKey(pk_id2);
        database.deleteKey(pk_id2);
        database.deleteKey(pk_id2);
        database.deleteCertificate(app_cert_id1);
        database.deleteCertificate(app_cert_id1);
        database.deleteCertificate(app_cert_id1);
        database.deleteLayout(layout_id, group_name, domain);
    }

    TEST_F(StorageTest, bool_setting) {
        const auto* const domain       = "my_domain";
        const auto* const bool_setting = "my_bool_setting";
        settings.defineSettingDomain(domain,
                                     {
                                         std::make_shared<magnesia::BooleanSetting>(bool_setting, "My Bool Setting",
                                                                                    "This is a boolean setting", false),
                                     });

        EXPECT_FALSE(settings.getBoolSetting({.name = bool_setting, .domain = domain}).value());
        settings.setBooleanSetting({.name = bool_setting, .domain = domain}, true);
        EXPECT_TRUE(settings.getBoolSetting({.name = bool_setting, .domain = domain}).value());
        settings.resetSetting({.name = bool_setting, .domain = domain});
        EXPECT_FALSE(settings.getBoolSetting({.name = bool_setting, .domain = domain}).value());
    }

    TEST_F(StorageTest, int_setting) {
        const auto* const domain        = "my_domain";
        const auto* const int_setting   = "my_int_setting";
        const int         default_value = 42;
        settings.defineSettingDomain(
            domain, {
                        std::make_shared<magnesia::IntSetting>(int_setting, "My Int Setting", "This is an int setting",
                                                               default_value, 0, 123),
                    });

        EXPECT_EQ(default_value, settings.getIntSetting({.name = int_setting, .domain = domain}));
        const int new_setting = 120;
        settings.setIntSetting({.name = int_setting, .domain = domain}, new_setting);
        EXPECT_EQ(new_setting, settings.getIntSetting({.name = int_setting, .domain = domain}));
        settings.resetSetting({.name = int_setting, .domain = domain});
        EXPECT_EQ(default_value, settings.getIntSetting({.name = int_setting, .domain = domain}));
    }

    TEST_F(StorageTest, string_setting) {
        const auto* const domain         = "my_domain";
        const auto* const string_setting = "my_string_setting";
        const auto* const default_value  = "default string";
        settings.defineSettingDomain(
            domain, {
                        std::make_shared<magnesia::StringSetting>(string_setting, "My String Setting",
                                                                  "This is a string setting", default_value),
                    });

        EXPECT_EQ(default_value, settings.getStringSetting({.name = string_setting, .domain = domain}));
        const auto* const new_setting = "new value";
        settings.setStringSetting({.name = string_setting, .domain = domain}, QString{new_setting});
        EXPECT_EQ(new_setting, settings.getStringSetting({.name = string_setting, .domain = domain}));
        settings.resetSetting({.name = string_setting, .domain = domain});
        EXPECT_EQ(default_value, settings.getStringSetting({.name = string_setting, .domain = domain}));
    }

    TEST_F(StorageTest, double_setting) {
        const auto* const domain         = "my_domain";
        const auto* const double_setting = "my_double_setting";
        const double      default_value  = 12.0;
        settings.defineSettingDomain(domain, {
                                                 std::make_shared<magnesia::DoubleSetting>(
                                                     double_setting, "My Double Setting", "This is a double setting",
                                                     default_value, -10.0, 420.0),
                                             });

        EXPECT_EQ(default_value, settings.getDoubleSetting({.name = double_setting, .domain = domain}));
        const double new_setting = 55.0;
        settings.setDoubleSetting({.name = double_setting, .domain = domain}, new_setting);
        EXPECT_EQ(new_setting, settings.getDoubleSetting({.name = double_setting, .domain = domain}));
        settings.resetSetting({.name = double_setting, .domain = domain});
        EXPECT_EQ(default_value, settings.getDoubleSetting({.name = double_setting, .domain = domain}));
    }

    TEST_F(StorageTest, enum_setting) {
        const auto* const domain        = "my_domain";
        const auto* const enum_setting  = "my_enum_setting";
        const auto* const default_value = "green";
        settings.defineSettingDomain(domain, {
                                                 std::make_shared<magnesia::EnumSetting>(
                                                     enum_setting, "My Enum Setting", "This is a enum setting",
                                                     magnesia::EnumSettingValue{default_value},
                                                     std::set{
                                                         magnesia::EnumSettingValue{"green"},
                                                         magnesia::EnumSettingValue{"blue"},
                                                         magnesia::EnumSettingValue{"red"},
                                                         magnesia::EnumSettingValue{"orange"},
                                                     }),
                                             });

        EXPECT_EQ(default_value, settings.getEnumSetting({.name = enum_setting, .domain = domain}));
        const auto* const new_setting = "orange";
        settings.setEnumSetting({.name = enum_setting, .domain = domain}, magnesia::EnumSettingValue{new_setting});
        EXPECT_EQ(new_setting, settings.getEnumSetting({.name = enum_setting, .domain = domain}));
        settings.resetSetting({.name = enum_setting, .domain = domain});
        EXPECT_EQ(default_value, settings.getEnumSetting({.name = enum_setting, .domain = domain}));
    }

    TEST_F(StorageTest, kv_setting) {
        const auto* const domain       = "some-domain";
        const auto* const key          = "my-key";
        const auto* const first_value  = "my-value";
        const auto* const second_value = "some-other-value";

        database.setKV(key, domain, first_value);
        EXPECT_EQ(first_value, database.getKV(key, domain).value());
        database.setKV(key, domain, second_value);
        EXPECT_EQ(second_value, database.getKV(key, domain).value());
        database.deleteKV(key, domain);
        EXPECT_FALSE(database.getKV(key, domain).has_value());
    }

    // NOLINTEND(bugprone-unchecked-optional-access, cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
} // namespace magnesia
