//Project
#include "confelement.h"
#include "cgt/CGTShare.h"

//STL

//Native

//Qt

ConfElement::ConfElement(const QString &pathFileConf)
    : m_pathFileConf(pathFileConf)
{
    parseSections();
    parseType();
    parseProperty();
}

void ConfElement::parseSections()
{
    QFile file(m_pathFileConf);
    file.open(QIODevice::ReadOnly);
    const QString data = QString::fromLocal8Bit(file.readAll());
    file.close();

    TypeSection section = ts_undefine;

    for (QString line : data.split("\r\n")) {
        line = line.trimmed();
        size_t size = line.size();
        if (size < 2)
            continue;

        if (line.at(0) == QLatin1Char('[')) {
            const QChar c = line.at(1).toLower();

            if (c == QLatin1Char('a')) {
                section = ts_about;
            } else if (c == QLatin1Char('t')) {
                section = ts_type;
            } else if (c == QLatin1Char('e')) {
                section = ts_edit;
            } else if (c == QLatin1Char('p')) {
                section = ts_property;
            } else if (c == QLatin1Char('m')) {
                section = ts_methods;
            } else {
                section = ts_undefine;
            }

            continue;
        }

        switch (section) {
        case ts_about:
            m_secAbout << line;
            break;
        case ts_type:
            m_secType << line;
            break;
        case ts_edit: {
            if (!m_editClass.isEmpty())
                continue;
            if (line.section('=', 0, 0).toLower() == QLatin1String("class"))
                m_editClass = line.section('=', 1, 1);
            break;
        }
        case ts_property:
            m_secProperty << line;
            break;
        case ts_methods:
            m_secMethod << line;
            break;
        default:
            break;
        }
    }
}

void ConfElement::parseAbout()
{

}

void ConfElement::parseType()
{
    for (const QString &line : m_secType) {
        QString sec0 = line.section("=", 0, 0).toLower();
        QString sec1 = line.section("=", 1, 1);
        if (sec0 == QLatin1String("class")) {
            m_class = sec1;
        } else if (sec0 == QLatin1String("inherit")) {
            m_inherit = sec1;
        } else if (sec0 == QLatin1String("sub")) {
            m_sub = sec1;
        } else if (sec0 == QLatin1String("info")) {
            m_info = sec1;
        } else if (sec0 == QLatin1String("icon")) {
            m_icon = sec1;
        } else if (sec0 == QLatin1String("view")) {
            m_view = sec1;
        } else if (sec0 == QLatin1String("tab")) {
            m_tab = sec1;
        } else if (sec0 == QLatin1String("interfaces")) {
            m_interfaces = sec1;
        }
    }
}

void ConfElement::parseProperty()
{
    //##имя_группы=описание
    //## закрываем группу

    //+ свойсво по-умолчанию, т.е. открывается редактор
    //при двойном клике.

    //@ Позволяет активировать свойство в виде метода с префиксом do*

    bool beginGroup = false;
    bool endGroup = false;
    for (const QString &line : m_secProperty) {
        bool makePoint = false;
        bool activated = false;
        bool equalSign = false;
        bool beginGroupLine = false;
        uchar countPipe = 0;
        uchar countSharp = 0;
        QString nameGroup;
        QString descGroup;
        QString nameProp;
        QString descProp;

        QString strType;
        QString value;

        QString buffer;
        size_t outIndex = line.size();

        for (size_t i = 0; i <= outIndex; ++i) {
            if (i == outIndex) {
                if (beginGroupLine) {
                    m_group << SharedConfPropGroup::create(nameGroup, descGroup);
                    continue;
                }
                SharedConfProp prop = SharedConfProp::create();
                prop->activated = activated;
                prop->makePoint = makePoint;
                prop->name = nameProp;
                prop->value = value;

                m_properties << prop;
                continue;
            }
            const QChar &c = line[i];

            if (c == QLatin1Char('#')) {
                ++countSharp;
                continue;
            }
            if (c == QLatin1Char('@')) {
                activated = true;
                continue;
            }
            if (c == QLatin1Char('+')) {
                makePoint = true;
                continue;
            }
            if (c == QLatin1Char('=')) {
                equalSign = true;
                continue;
            }
            if (c == QLatin1Char('|')) {
                ++countPipe;
                continue;
            }

            if (countSharp == 2) {
                if (!beginGroup) {
                    beginGroup = true;
                    beginGroupLine = true;
                }
            }

            if (beginGroupLine) {
                if (!equalSign)
                    nameGroup += c;
                else
                    descGroup += c;

                continue;
            }

            if (countPipe) {
                if (countPipe == 1)  //Тип
                    strType += c;
                if (countPipe == 2)  //Значение
                    value += c;

                continue;
            }

            if (!equalSign)
                nameProp += c;
            else
                descProp += c;
        }

        qInfo() << "test";
    }
}