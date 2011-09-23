/***************************************************************************
  textselection  - widget to select header- and footer text data for the doc
                             -------------------
    begin                : 2007-06-01
    copyright            : (C) 2007 by Klaas Freitag
    email                : freitag@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "textselection.h"
#include "filterheader.h"
#include "defaultprovider.h"
#include "kraftdoc.h"
#include "doctype.h"

#include <klocale.h>
#include <kdebug.h>

#include <kdialog.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kiconloader.h>

#include <QtGui>

TextSelection::TextSelection( QWidget *parent, KraftDoc::Part part )
  :QWidget( parent ),
    mPart( part )
{
  QGroupBox *groupBox = new QGroupBox(tr("Template Collection"));

  QVBoxLayout *layout = new QVBoxLayout;
  setLayout(layout);

  layout->setMargin( KDialog::marginHint() );
  layout->setSpacing( KDialog::spacingHint() );
  layout->addWidget( groupBox );

  /* a view for the entry text repository */
  QVBoxLayout *vbox = new QVBoxLayout;

  mHeadLabel = new QLabel( i18n( "%1 Templates" ).arg( KraftDoc::partToString( mPart ) ));
  vbox->addWidget( mHeadLabel );

  mTextNameView = new QListView;
  vbox->addWidget(mTextNameView);
  mTextNameView->setSelectionMode( QAbstractItemView::SingleSelection );
  mTextNameView->setMaximumHeight(60 );

  connect( mTextNameView, SIGNAL(clicked(QModelIndex)),
           this, SLOT(slotNameSelected(QModelIndex)));
  connect( mTextNameView, SIGNAL(doubleClicked(QModelIndex)),
           this, SLOT(slotNameDoubleClicked(QModelIndex)));

  mTextDisplay = new QTextEdit;
  mTextDisplay->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
  mTextDisplay->setLineWidth( 1 );
  mTextDisplay->setReadOnly(true);
  QPalette p = mTextDisplay->palette();
  p.setColor( QPalette::Active, QPalette::Base, p.color(QPalette::Window));
  mTextDisplay->setPalette(p);
  vbox->addWidget( mTextDisplay, 3 );

  mHelpDisplay = new QLabel;
  mHelpDisplay->setStyleSheet("background-color: #ffcbcb;");
  mHelpDisplay->setAutoFillBackground(true);
  // QMargins m( KDialog::marginHint(), KDialog::marginHint(), KDialog::marginHint(), KDialog::marginHint() );
  // mHelpDisplay->setContentsMargins( m );
  mHelpDisplay->setWordWrap( true );
  // mHelpDisplay->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
  mHelpDisplay->setMinimumHeight( 80 );
  mHelpDisplay->setAlignment( Qt::AlignCenter | Qt::AlignVCenter );
  mHelpDisplay->hide();

  vbox->addWidget( mHelpDisplay );

  groupBox->setLayout( vbox );

  mTemplNamesModel = new QStringListModel;
  mTextNameView->setModel( mTemplNamesModel );
  connect( mTextNameView->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ),
           this, SLOT( slotTemplateNameSelected( const QModelIndex&, const QModelIndex& ) ) );

#if 0
  connect( mTextsView, SIGNAL( currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*) ),
           this, SLOT( slotSelectionChanged( QTreeWidgetItem* ) ) );
  connect( mTextsView, SIGNAL(doubleClicked(QModelIndex) ),
           this, SLOT( slotSelectionChanged( QTreeWidgetItem* ) ) );
#endif

  // Context Menu
  mMenu = new QMenu( this );
  mMenu->setTitle( i18n("Template Actions") );
#if 0
  mTextsView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect( mTextsView, SIGNAL(customContextMenuRequested(QPoint) ),
            this, SLOT( slotRMB( QPoint ) ) );
#endif

  initActions();
}

/* selected the name of a template in the listview of template names */
void TextSelection::slotTemplateNameSelected( const QModelIndex& current, const QModelIndex& )
{
  mCurrTemplateName = mTemplNamesModel->data( current, Qt::DisplayRole ).toString();
  kDebug() << "New selected document type: " << mCurrTemplateName;
  showHelp();

  DocText dt = currentDocText();
  showDocText( dt );
}

void TextSelection::showDocText( DocText dt )
{
  if( dt.type() != KraftDoc::Unknown && dt.isStandardText() ) {
    showHelp(i18n("This is the standard text used in new documents."));
  }

  mTextDisplay->setText( dt.text() );
}

void TextSelection::slotSelectDocType( const QString& doctype )
{
  QString partStr = KraftDoc::partToString( mPart );
  mHeadLabel->setText( QString( i18n( "%1 Templates for %2" ).arg( partStr ).arg(doctype) ) );
  mDocType = doctype;

  DocTextList dtList = DefaultProvider::self()->documentTexts( doctype, mPart );

  QStringList templNames;
  if( dtList.count() == 0 ) {
    showHelp( i18n("There is no %1 template text available for document type %2.<br/>"
                   "Click the add-button below to create one.").arg( partStr ).arg( doctype ) );
  } else {
    foreach( DocText dt, dtList ) {
      templNames << dt.name();
    }
    showHelp();
  }
  mTemplNamesModel->setStringList( templNames );
}

void TextSelection::addNewDocText( const DocText& )
{
  slotSelectDocType( mDocType );
  QModelIndex selected;
  mTextNameView->selectionModel()->setCurrentIndex( selected, QItemSelectionModel::Select);
}

/* requires the QListViewItem set as a member in the doctext */
void TextSelection::updateDocText( const DocText& dt )
{
  QModelIndex selected = mTextNameView->selectionModel()->currentIndex();
  slotSelectDocType( mDocType );
  mTextNameView->selectionModel()->setCurrentIndex( selected, QItemSelectionModel::Select );
}

void TextSelection::deleteCurrentText()
{
  slotSelectDocType( mDocType );
}


TextSelection::~TextSelection()
{
}

void TextSelection::initActions()
{
  mActions     = new KActionCollection( this );
  mAcMoveToDoc = mActions->addAction( "moveToDoc", this, SIGNAL(actionCurrentTextToDoc()));
  mAcMoveToDoc->setIcon( KIcon( "go-previous" ));
  mAcMoveToDoc->setText( i18n("&Use in Document") );

  mMenu->addAction( mAcMoveToDoc );

}

/* if the help string is empty, the help widget disappears. */
void TextSelection::showHelp( const QString& help )
{
  mHelpDisplay->setText( help );
  if( help.isEmpty() ) {
    mHelpDisplay->hide();
  } else {
    mHelpDisplay->show();
#if 0
    kDebug() << "Displaying help text: " << help;

    QPropertyAnimation *ani = new QPropertyAnimation( mHelpDisplay, "geometry" );
    QRect r2 = r1;
    r2.setHeight( 200 );
    ani->setDuration( 2000 );
    ani->setStartValue( r1 );
    ani->setEndValue( r2 );
    ani->start();
#endif
  }
}

DocText TextSelection::currentDocText() const
{
  DocTextList dtList = DefaultProvider::self()->documentTexts( mDocType, mPart );
  foreach( DocText dt, dtList ) {
    if( dt.name() == mCurrTemplateName ) {
      return dt;
    }
  }
  DocText dt;
  return dt;
}

QString TextSelection::currentText() const
{
  return currentDocText().text();
}


void TextSelection::slotRMB(QPoint point )
{
  // mMenu->popup( mTextsView->mapToGlobal(point) );
}


