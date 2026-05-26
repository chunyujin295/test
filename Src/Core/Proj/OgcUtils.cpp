

#include <QColor>
#include <QStringList>
#include <QTextStream>
#include <QObject>
#include <QRegularExpression>








#include "Core/Proj/OgcUtils.h"
namespace AI3D
{
    namespace PROJ
    {
        OgcCrsUtils::CRSFlavor OgcCrsUtils::parseCrsName(const QString& crsName, QString& authority, QString& code)
        {
            const thread_local QRegularExpression re_url(QRegularExpression::anchoredPattern(QStringLiteral("http://www\\.opengis\\.net/gml/srs/epsg\\.xml#(.+)")), QRegularExpression::CaseInsensitiveOption);
            const QRegularExpressionMatch match1 = re_url.match(crsName);
            if (match1.hasMatch())
            {
                authority = QStringLiteral("EPSG");
                code = match1.captured(1);
                return CRSFlavor::HTTP_EPSG_DOT_XML;
            }

            const thread_local QRegularExpression re_ogc_urn(QRegularExpression::anchoredPattern(QStringLiteral("urn:ogc:def:crs:([^:]+).+(?<=:)([^:]+)")), QRegularExpression::CaseInsensitiveOption);
            const QRegularExpressionMatch match2 = re_ogc_urn.match(crsName);
            if (match2.hasMatch())
            {
                authority = match2.captured(1);
                code = match2.captured(2);
                return CRSFlavor::OGC_URN;
            }

            const thread_local QRegularExpression re_x_ogc_urn(QRegularExpression::anchoredPattern(QStringLiteral("urn:x-ogc:def:crs:([^:]+).+(?<=:)([^:]+)")), QRegularExpression::CaseInsensitiveOption);
            const QRegularExpressionMatch match3 = re_x_ogc_urn.match(crsName);
            if (match3.hasMatch())
            {
                authority = match3.captured(1);
                code = match3.captured(2);
                return CRSFlavor::X_OGC_URN;
            }

            const thread_local QRegularExpression re_http_uri(QRegularExpression::anchoredPattern(QStringLiteral("http://www\\.opengis\\.net/def/crs/([^/]+).+/([^/]+)")), QRegularExpression::CaseInsensitiveOption);
            const QRegularExpressionMatch match4 = re_http_uri.match(crsName);
            if (match4.hasMatch())
            {
                authority = match4.captured(1);
                code = match4.captured(2);
                return CRSFlavor::OGC_HTTP_URI;
            }

            const thread_local QRegularExpression re_auth_code(QRegularExpression::anchoredPattern(QStringLiteral("([^:]+):(.+)")), QRegularExpression::CaseInsensitiveOption);
            const QRegularExpressionMatch match5 = re_auth_code.match(crsName);
            if (match5.hasMatch())
            {
                authority = match5.captured(1);
                code = match5.captured(2);
                return CRSFlavor::AUTH_CODE;
            }

            return CRSFlavor::UNKNOWN;
        }
    }
}