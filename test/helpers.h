#ifndef TEST_KPMHELPERS_H
#define TEST_KPMHELPERS_H

class QString;

class KPMCoreInitializer
{
public:
    KPMCoreInitializer();
    KPMCoreInitializer(const QString& backend);
    KPMCoreInitializer(const char *backend);

    bool isValid() const { return m_isValid; }
private:
    bool m_isValid;
} ;

#endif
