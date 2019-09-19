#include "pagegridwidget.h"

PageGridWidget::PageGridWidget(QWidget *parent) : QWidget(parent)
{
    mainLayout = new QVBoxLayout();

    dragEventFilter = new DragEventFilter(this);

    scrollArea = new QScrollArea();
    /*scrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    scrollArea->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded);*/

    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollAreaWidget = new QFrame();
    gridLayout = new QGridLayout();
    scrollAreaWidget->setLayout(gridLayout);
    scrollArea->setWidget(scrollAreaWidget);

    mainLayout->addWidget(scrollArea);

    setLayout(mainLayout);

    connect(reinterpret_cast<QAbstractSlider*>(scrollArea->verticalScrollBar()), SIGNAL(valueChanged(int)), this, SLOT(onScrollbarChange(int)));
}

PageGridWidget::~PageGridWidget()
{
    delete gridLayout;
    delete scrollAreaWidget;
    delete scrollArea;
    delete mainLayout;
}

void PageGridWidget::displayDocPages(PdfUtil *doc)
{
    currDoc = doc;

    removeAllWidgets();

    int rows = 0, columns = 0;

    for(int page = 0; page < doc->GetPageCount(); page++)
    {
        QLabel* pic = new QLabel();

        pic->installEventFilter(dragEventFilter);

        //delete pageImage;
        displayedPictures.append(pic);

        pic->setMaximumSize(pic->size());

        pic->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

        gridLayout->addWidget(pic, rows, columns);
        //gridLayout->addWidget(pic);

        if(++columns >= columnsPerRow)
        {
            ++rows;
            columns = 0;
        }
    }

    gridLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    gridLayout->setSpacing(25);

    repaint();

    renderedPages.resize(doc->GetPageCount());
    renderedPages.fill(false);

    for(int n = 0; n < std::min(displayedPictures.length(), 15); n++)
    {
        QLabel* pic = displayedPictures.at(n);
        if(pic->pixmap() == nullptr)
        {
            auto renderedPage = currDoc->GetPdfRenderedPage(n);

            pic->setMinimumSize(renderedPage->getWidth(), renderedPage->getHeight());
            auto pageImage = renderedPage->getImage();

            QPixmap p = QPixmap::fromImage(*pageImage);

            if(renderedPage->getWidth() > renderedPage->getHeight())
                 //loadedPixmaps.append(QPixmap::fromImage(*pageImage).scaledToWidth(300, Qt::TransformationMode::SmoothTransformation));
                p = p.scaledToWidth(300, Qt::TransformationMode::SmoothTransformation);
            else
                p = p.scaledToHeight(300, Qt::TransformationMode::SmoothTransformation);
                 //loadedPixmaps.append(QPixmap::fromImage(*pageImage).scaledToHeight(300, Qt::TransformationMode::SmoothTransformation));

            delete pageImage;
            delete renderedPage;

            pic->setPixmap(p);
            pic->setFixedSize(p.size());

        }
    }

    lastDisplayRange = QPair<int,int>(0, std::min(displayedPictures.length(), 15));
}

void PageGridWidget::onScrollbarChange(int value)
{
    qDebug() << "Scrollbar changed";

    repaintThumbnails();
}

void PageGridWidget::onTimerEnd(int pageNum)
{
    displayedPictures.at(pageNum)->setStyleSheet("border:none;");
}

void PageGridWidget::removeAllWidgets()
{
    for (QWidget* label : displayedPictures)
        gridLayout->removeWidget(label);

    qDeleteAll(displayedPictures);
    displayedPictures.clear();

    loadedPixmaps.clear();
}

bool PageGridWidget::compareDisplayRanges(QPair<int,int>& displayRange, QPair<int,int>& prefDisplayRange)
{
    return (displayRange.first >= prefDisplayRange.first + 6) &&
            (displayRange.second <= prefDisplayRange.second - 6);
}

QPair<int, int> PageGridWidget::getPreferredDisplayRange()
{
    auto dispRange = getDisplayRange();
    dispRange.first = std::max(dispRange.first - 12, 0);
    dispRange.second = std::min(dispRange.second + 12, displayedPictures.length()-1);
    return dispRange;
}

QPair<int, int> PageGridWidget::getDisplayRange()
{
    QSize currentSize = size();
    int toDisplay = std::ceil(currentSize.height()/350.0)*3;
    int firstDisplayed = 0;
    for(int i = 0; i < displayedPictures.length(); i++)
        if(!displayedPictures.at(i)->visibleRegion().isNull())
        {
            firstDisplayed = i;
            break;
        }

    int lastDisplayed = std::min(firstDisplayed+toDisplay, displayedPictures.length()-1);
    return QPair<int, int>(firstDisplayed, lastDisplayed);
}

void PageGridWidget::repaintThumbnails()
{
    auto displayRange = getDisplayRange();
    if(!compareDisplayRanges(displayRange, lastDisplayRange))
    {
        qDebug() << "Repainting thumbs";
        lastDisplayRange = getPreferredDisplayRange();

        for(int n = 0; n < displayedPictures.length(); n++)
        {
            QLabel* pic = displayedPictures.at(n);
            if(n >= lastDisplayRange.first && n <= lastDisplayRange.second)
            {
                if(pic->pixmap() == nullptr)
                {
                    auto renderedPage = currDoc->GetPdfRenderedPage(n);

                    pic->setMinimumSize(renderedPage->getWidth(), renderedPage->getHeight());
                    auto pageImage = renderedPage->getImage();

                    QPixmap p = QPixmap::fromImage(*pageImage);

                    if(renderedPage->getWidth() > renderedPage->getHeight())
                        p = p.scaledToWidth(300, Qt::TransformationMode::SmoothTransformation);
                    else
                        p = p.scaledToHeight(300, Qt::TransformationMode::SmoothTransformation);

                    delete pageImage;
                    delete renderedPage;

                    pic->setPixmap(p);
                    pic->setFixedSize(p.size());

                }
            } else
                pic->clear();
        }
    }
}

int PageGridWidget::indexOf(QLabel *lbl)
{
    for(int i = 0; i < displayedPictures.length(); i++)
        if(displayedPictures.at(i) == lbl)
            return i;

    return -1;
}

PdfUtil *PageGridWidget::getCurrentDoc()
{
    return currDoc;
}

void PageGridWidget::navigateToPage(int pageNum)
{
    if(pageNum >= 0 && pageNum < displayedPictures.length())
    {
        auto lbl = displayedPictures.at(pageNum);
        scrollArea->ensureWidgetVisible(lbl);

        lbl->setStyleSheet("border: 3px solid red;");
        DelayedAction* delayedAction = new DelayedAction(pageNum);
        connect(delayedAction, SIGNAL(delayEnd(int)), this, SLOT(onTimerEnd(int)));
        delayedAction->start(2500);
    }
}

void PageGridWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    qDebug() << "Resized";
    repaintThumbnails();
}