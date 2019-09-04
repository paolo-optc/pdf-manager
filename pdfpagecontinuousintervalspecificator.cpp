#include "pdfpagecontinuousintervalspecificator.h"

PdfPageContinuousIntervalSpecificator::PdfPageContinuousIntervalSpecificator(int firstPage, int lastPage)
{
    if(firstPage < lastPage)
    {
        lowerPage = firstPage;
        higherPage = lastPage;
    } else if (firstPage > lastPage){
        lowerPage = lastPage;
        higherPage = firstPage;
    } else lowerPage = higherPage = firstPage;
}

PdfPageContinuousIntervalSpecificator::PdfPageContinuousIntervalSpecificator(QString descriptor)
{
    QRegularExpressionMatch rMatch = regxp.match(descriptor);
    if(rMatch.isValid())
    {
        int firstPage = (rMatch.captured(1).toInt()-1);
        int lastPage = (rMatch.captured(2).toInt()-1);

        if(firstPage < lastPage)
        {
            lowerPage = firstPage;
            higherPage = lastPage;
        } else if (firstPage > lastPage){
            lowerPage = lastPage;
            higherPage = firstPage;
        } else lowerPage = higherPage = firstPage;
    }
}

PdfPageContinuousIntervalSpecificator::~PdfPageContinuousIntervalSpecificator()
{

}

QList<int> PdfPageContinuousIntervalSpecificator::getAllPages()
{
    QList<int> pagesList;
    for (int i = lowerPage; i <= higherPage; i++)
        pagesList << i;
    return pagesList;
}

QString PdfPageContinuousIntervalSpecificator::getDisplayText()
{
    return QString("%1 - %2").arg(QString::number(lowerPage+1), QString::number(higherPage+1));
}