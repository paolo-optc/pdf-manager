#ifndef PDFUTIL_H
#define PDFUTIL_H

#include <QVector>
#include <QImage>
#include <QString>
#include <QException>
//#include <podofo/podofo.h>
#include <mupdf/fitz.h>

#include "pdfrenderedpage.h"
#include "pdfexception.h"

class PdfUtil
{
public:
    PdfUtil(QString path);
    ~PdfUtil();
    int GetPageCount() const;
    //PoDoFo::PdfMemDocument* getPdfDocument();
    PdfRenderedPage*  GetPdfRenderedPage(int pageNum);
    class PdfNewDocument;
private:
    QString docPath;
    fz_context* pdf_ctx = nullptr;
    int pageCount = 0;
    //PoDoFo::PdfMemDocument* pdfDoc = nullptr;
    fz_document* pdf_doc = nullptr;
    fz_pixmap* pdf_pixmap = nullptr;
    fz_matrix pdf_matrix;
    int zoom = 100;
    int rotation = 0;
};

#endif // PDFUTIL_H