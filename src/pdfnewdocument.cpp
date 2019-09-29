#include "pdfnewdocument.h"

PdfUtil::PdfNewDocument::PdfNewDocument(QString fileName)
    :
      fileName(fileName)
{
    ctx = fz_new_context(nullptr, nullptr, FZ_STORE_UNLIMITED);
    if(!ctx)
        throw PdfException("Failed to create context.");

    pdf_parse_write_options(ctx, &opts, flags.toLocal8Bit());

    fz_try(ctx)
    {
        doc_des = pdf_create_document(ctx);
    }
    fz_catch(ctx)
    {
        fz_flush_warnings(ctx);
        fz_drop_context(ctx);
        throw PdfException("Cannot create PDF document.");
    }
}

PdfUtil::PdfNewDocument::~PdfNewDocument()
{
    pdf_drop_document(ctx, doc_des);
    fz_drop_context(ctx);
}

void PdfUtil::PdfNewDocument::addPagesFromRange(PdfPageRangeSpecificator* range)
{
    pdf_document* doc_src = pdf_open_document(ctx, range->getDocumentPath().toLocal8Bit());

    for(int page : range->getAllPages())
    {
        graft_map = pdf_new_graft_map(ctx, doc_des);
        addPageFromParent(doc_src, page);
        pdf_drop_graft_map(ctx, graft_map);
    }

    pdf_drop_document(ctx, doc_src);
}

void PdfUtil::PdfNewDocument::addPageFromParent(pdf_document* doc_src, int pageNum)
{
    pdf_obj *page_ref;
    pdf_obj *page_dict = NULL;
    pdf_obj *obj;
    pdf_obj *ref = NULL;
    int i;

    /* Copy as few key/value pairs as we can. Do not include items that reference other pages. */
    static pdf_obj * const copy_list[] = {
        PDF_NAME(Contents),
        PDF_NAME(Resources),
        PDF_NAME(MediaBox),
        PDF_NAME(CropBox),
        PDF_NAME(BleedBox),
        PDF_NAME(TrimBox),
        PDF_NAME(ArtBox),
        PDF_NAME(Rotate),
        PDF_NAME(UserUnit)
    };

    fz_var(ref);
    fz_var(page_dict);

    fz_try(ctx)
    {
        page_ref = pdf_lookup_page_obj(ctx, doc_src, pageNum);
        pdf_flatten_inheritable_page_items(ctx, page_ref);

        /* Make a new page object dictionary to hold the items we copy from the source page. */
        page_dict = pdf_new_dict(ctx, doc_des, 4);

        pdf_dict_put(ctx, page_dict, PDF_NAME(Type), PDF_NAME(Page));

        for (i = 0; i < nelem(copy_list); i++)
        {
            obj = pdf_dict_get(ctx, page_ref, copy_list[i]);
            if (obj != NULL)
                pdf_dict_put_drop(ctx, page_dict, copy_list[i], pdf_graft_mapped_object(ctx, graft_map, obj));
        }

        /* Add the page object to the destination document. */
        ref = pdf_add_object(ctx, doc_des, page_dict);

        /* Insert it into the page tree. */
        pdf_insert_page(ctx, doc_des, addedPages, ref);
        addedPages++;
    }
    fz_always(ctx)
    {
        pdf_drop_obj(ctx, page_dict);
        pdf_drop_obj(ctx, ref);
    }
    fz_catch(ctx)
    {
        throw PdfException("Can't add page "+QString::number(pageNum)+" to PDF file.");
    }
}

void PdfUtil::PdfNewDocument::Save()
{
    fz_try(ctx)
        pdf_save_document(ctx, doc_des, fileName.toLocal8Bit(), &opts);
    fz_catch(ctx)
        throw PdfException("Can't save to path: "+fileName);
}