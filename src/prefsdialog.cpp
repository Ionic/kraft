/***************************************************************************
                   prefsdialog.cpp  - the preferences Dialog
                             -------------------
    begin                : Sun Jul 3 2004
    copyright            : (C) 2004 by Klaas Freitag
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

#include<qlayout.h>
#include<qlineedit.h>
#include <qlineedit.h>
#include<qlabel.h>
#include<qframe.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qtextedit.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qlistbox.h>
#include <qsqlquery.h>
#include <qspinbox.h>

#include<kdialog.h>
#include<klocale.h>
#include<kiconloader.h>
#include<kmessagebox.h>

#include "prefsdialog.h"
#include "katalogsettings.h"
#include "kraftsettings.h"
#include "kraftdb.h"
#include "kraftdoc.h"
#include "defaultprovider.h"
#include "doctype.h"
#include "doctypeedit.h"
#include <kinputdialog.h>



// --------------------------------------------------------------------------------

DocTypeEdit::DocTypeEdit( QWidget *parent )
  : DocTypeEditBase( parent )
{
  connect( mTypeListBox, SIGNAL( highlighted( const QString& ) ),
           this,  SLOT( slotDocTypeSelected( const QString& ) ) );

  QStringList types = DocType::allLocalised();;
  mTypeListBox->clear();
  mTypeListBox->insertStringList( types );

  for ( QStringList::Iterator it = types.begin(); it != types.end(); ++it ) {
    mOrigDocTypes[*it] = DocType( *it );
  }

  mTypeListBox->setSelected( 0, true );

  mPbAdd->setPixmap( BarIcon( "filenew" ) );
  mPbEdit->setPixmap( BarIcon( "edit" ) );
  mPbRemove->setPixmap( BarIcon( "editdelete" ) );

  connect( mPbAdd, SIGNAL( clicked() ),
           SLOT( slotAddDocType() ) );
  connect( mPbEdit, SIGNAL( clicked() ),
           SLOT( slotEditDocType() ) );
  connect( mPbRemove, SIGNAL( clicked() ),
           SLOT( slotRemoveDocType() ) );
}

void DocTypeEdit::slotAddDocType()
{
  kdDebug() << "Adding a doctype!" << endl;

  QString newName = KInputDialog::getText( i18n( "Add Document Type" ),
                                           i18n( "Enter the name of a new document type" ) );
  if ( newName.isEmpty() ) return;
  kdDebug() << "New Name to add: " << newName << endl;

  if ( mTypeListBox->findItem( newName ) ) {
    kdDebug() << "New Name already exists" << endl;
  } else {
    mTypeListBox->insertItem( newName );
    mOrigDocTypes[newName] = DocType( newName );
    mAddedTypes.append( newName );
  }
}

void DocTypeEdit::slotEditDocType()
{
  kdDebug() << "Editing a doctype!" << endl;

  QString currName = mTypeListBox->currentText();

  if ( currName.isEmpty() ) return;

  QString newName = KInputDialog::getText( i18n( "Add Document Type" ),
                                           i18n( "Enter the name of a new document type" ),
                                           currName );
  if ( newName.isEmpty() ) return;
  kdDebug() << "edit: " << currName << " became " << newName << endl;
  if ( newName != currName ) {
    mTypeListBox->changeItem( newName, mTypeListBox->currentItem() );

    /* check if the word that was changed now was already changed before. */
    bool prechanged = false;
    bool skipEntry = false;
    QMap<QString, QString>::Iterator it;
    for ( it = mTypeNameChanges.begin(); !prechanged && it != mTypeNameChanges.end(); ++it ) {

      if (it.key() == currName ) { // it was changed back to an original name.
        mTypeNameChanges.remove( it );
        skipEntry = true;
      }

      if ( !skipEntry && it.data() == currName ) {
        kdDebug() << "Was changed before, key is " << it.key() << endl;
        currName = it.key();
        prechanged = true;
      }
    }
    if ( ! skipEntry ) {
      mTypeNameChanges[currName] = newName;
    }
  }
}

void DocTypeEdit::slotRemoveDocType()
{
  kdDebug() << "Removing a doctype!" << endl;

  QString currName = mTypeListBox->currentText();

  if ( currName.isEmpty() ) {
    kdDebug() << "No current Item, return" << endl;
    return;
  }

  if ( mAddedTypes.find( currName ) != mAddedTypes.end() ) {
    // remove item from recently added list.
    mAddedTypes.remove( currName );
    mOrigDocTypes.remove( currName );
  } else {
    QString toRemove = currName;
    QMap<QString, QString>::Iterator it;
    for ( it = mTypeNameChanges.begin(); it != mTypeNameChanges.end(); ++it ) {
      if ( currName == it.data() ) {
        // remove the original name
        toRemove = it.key(); // the original name
      }
    }
    mRemovedTypes.append( toRemove );
  }
  mTypeListBox->removeItem( mTypeListBox->currentItem() );
}

void DocTypeEdit::slotDocTypeSelected( const QString& newValue )
{
  QString value = mTypeListBox->currentText();
  if ( ! newValue.isEmpty() ) {
    value = newValue;
  }
  DocType dt( value );

  kdDebug() << "Selected doc type " << value << endl;
  mIdent->setText( dt.identTemplate() );
  int nextNum = dt.nextIdentId( false );
  mCounter->setText( QString::number( nextNum ) );
  mNumCycle->setText( dt.numberCycleName() );
  mHeader->setText( i18n( "Details for %1:" ).arg( dt.name() ) );
  mExampleId->setText( dt.generateDocumentIdent( 0, nextNum ) );

#if 0
  QSqlQuery q;
  q.prepare( "SELECT lastIdentNumber FROM numberCycles WHERE name=:name" );


  int num = -1;
  q.bindValue( ":name", dt.numberCycleName() );
  q.exec();
  if ( q.next() ) {
    num = 1+( q.value( 0 ).toInt() );
  }

  mCounterEdit->setValue( num );
  mNumberCycleCombo->setCurrentText( dt.numberCycleName() );
  QString example;
  if ( num > -1 ) {
    example = dt.generateDocumentIdent( 0, num );
    mCounterEdit->setMinValue( num );
  }
  mExampleId->setText( example );
#endif
}

QStringList DocTypeEdit::allNumberCycles()
{
  QStringList re;
  re << QString::fromLatin1( "default" );
  QSqlQuery q( "SELECT av.value FROM attributes a, attributeValues av "
               "WHERE a.id=av.attributeId AND a.hostObject='DocType' "
               "AND a.name='identNumberCycle'" );

  while ( q.next() ) {
    QString cycleName = q.value(0).toString();
    re << cycleName;
  }
  return re;
}

void DocTypeEdit::saveDocTypes()
{
  // removed doctypes
  for ( QStringList::Iterator it = mRemovedTypes.begin(); it != mRemovedTypes.end(); ++it ) {
    if ( mOrigDocTypes.contains( *it ) ) {
      DocType dt = mOrigDocTypes[*it];
      removeTypeFromDb( *it );
      mOrigDocTypes.remove( *it );
    }
  }

  // added doctypes
  for ( QStringList::Iterator it = mAddedTypes.begin(); it != mAddedTypes.end(); ++it ) {
    if ( mOrigDocTypes.contains( *it ) ) { // just to check
      QSqlQuery q;
      q.prepare( "INSERT INTO DocTypes (name) VALUES (:name)" );
      QString name = *it;
      q.bindValue( ":name", name.utf8() );
      q.exec();
      kdDebug() << "Created DocTypes-Entry " << *it << endl;
    }
  }

  // edited doctypes
  QMap<QString, QString>::Iterator it;
  for ( it = mTypeNameChanges.begin(); it != mTypeNameChanges.end(); ++it ) {
    QString oldName( it.key() );
    if ( mOrigDocTypes.contains( oldName ) ) {
      QString newName = it.data();
      kdDebug() << "Renaming " << oldName << " to " << newName << endl;
      DocType dt = mOrigDocTypes[oldName];
      dt.setName( newName );
      mOrigDocTypes.remove( oldName );
      mOrigDocTypes[newName] = dt;
      renameTypeInDb( oldName, newName );
    } else {
      kdError() << "Can not find doctype to change named " << oldName << endl;
    }
  }

  // now the list of document types should be up to date and reflected into
  // the database.
  DocType::clearMap();
}

void DocTypeEdit::removeTypeFromDb( const QString& name )
{
  QSqlQuery delQuery;

  dbID id = DocType::docTypeId( name );
  if ( !id.isOk() ) {
    kdDebug() << "Can not find doctype " << name << " to remove!" << endl;
    return;
  }

  // delete in DocTypeRelations
  delQuery.prepare( "DELETE FROM DocTypeRelations WHERE followerId=:id or typeId=:id" );
  delQuery.bindValue( ":id", id.toString() );
  delQuery.exec();

  // delete in DocTexts
  delQuery.prepare( "DELETE FROM DocTexts WHERE DocTypeId=:id" );
  delQuery.bindValue( ":id", id.toString() );
  delQuery.exec();

  // delete in the DocTypes table
  delQuery.prepare( "DELETE FROM DocTypes WHERE docTypeId=:id" );
  delQuery.bindValue( ":id", id.toString() );
  delQuery.exec();

  AttributeMap attMap( "DocType" );
  attMap.dbDeleteAll( id );
}

void DocTypeEdit::renameTypeInDb( const QString& oldName,  const QString& newName )
{
  QSqlQuery q;
  q.prepare( "UPDATE DocTypes SET name=:newName WHERE docTypeID=:oldId" );
  dbID id = DocType::docTypeId( oldName );
  if ( id.isOk() ) {
    q.bindValue( ":newName", newName.utf8() );
    q.bindValue( ":oldId", id.toInt() );
    q.exec();
    if ( q.numRowsAffected() == 0 ) {
      kdError() << "Database update failed for renaming " << oldName << " to " << newName << endl;
    } else {
      kdDebug() << "Renamed doctype " << oldName << " to " << newName << endl;
    }
  } else {
    kdError() << "Could not find the id for doctype named " << oldName << endl;
  }
}



// ################################################################################

PrefsDialog::PrefsDialog( QWidget *parent)
    : KDialogBase( IconList,  i18n("Configure Kraft"), Ok|Cancel, Ok, parent,
                   "PrefsDialog", true, true )
{
  databaseTab();
  docTab();
  doctypeTab();

  readConfig();
  slotCheckConnect();
}


void PrefsDialog::databaseTab()
{
  QLabel *label;
  QFrame *topFrame = addPage( i18n( "Database" ),
                              i18n( "Database Connection Settings" ),
                              DesktopIcon( "connect_no" ) ); // KDE 4 name: (probably) network-server-database

  QVBoxLayout *vboxLay = new QVBoxLayout( topFrame );
  QGridLayout *topLayout = new QGridLayout( topFrame );
  vboxLay->addLayout( topLayout );

  topLayout->setSpacing( spacingHint() );
  topLayout->setColSpacing( 0, spacingHint() );

  label = new QLabel(i18n("Database Host:"), topFrame );
  topLayout->addWidget(label, 0,0);

  label = new QLabel(i18n("Database Name:"), topFrame );
  topLayout->addWidget(label, 1,0);

  label = new QLabel(i18n("Database User:"), topFrame );
  topLayout->addWidget(label, 2,0);

  label = new QLabel(i18n("Database Password:"), topFrame );
  topLayout->addWidget(label, 3,0);

  label = new QLabel(i18n("Connection Status:"), topFrame );
  topLayout->addWidget(label, 4,0);

  m_pbCheck = new QPushButton( i18n( "Check Connection" ), topFrame );
  m_pbCheck->setEnabled( false );
  topLayout->addWidget( m_pbCheck, 5, 1 );

  QLabel *l1 = new QLabel(  i18n( "Please restart Kraft after "
                                  "changes in the database connection "
                                  "parameters to make the changes "
                                  "effective!" ), topFrame );
  l1->setTextFormat( Qt::RichText );
  l1->setBackgroundColor( QColor( "#ffcbcb" ) );
  l1->setMargin( 5 );
  l1->setFrameStyle( QFrame::Box + QFrame::Raised );
  l1->setLineWidth( 1 );
  l1->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak );
  topLayout->addMultiCellWidget( l1, 6,  6, 0, 1 );



  m_leHost = new QLineEdit( topFrame );
  connect( m_leHost, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( slotTextChanged( const QString& ) ) );
  topLayout->addWidget(m_leHost, 0,1);

  m_leName = new QLineEdit( topFrame );
  connect( m_leName, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( slotTextChanged( const QString& ) ) );
  topLayout->addWidget(m_leName, 1,1);

  m_leUser = new QLineEdit( topFrame );
  connect( m_leUser, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( slotTextChanged( const QString& ) ) );
  topLayout->addWidget(m_leUser, 2,1);

  m_lePasswd = new QLineEdit( topFrame );
  m_lePasswd->setEchoMode(QLineEdit::Password);
  connect( m_lePasswd, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( slotTextChanged( const QString& ) ) );
  topLayout->addWidget(m_lePasswd, 3,1);

  m_statusLabel = new QLabel( topFrame );
  topLayout->addWidget( m_statusLabel,  4, 1 );

  connect( m_pbCheck, SIGNAL( clicked() ),
           this, SLOT( slotCheckConnect() ) );

  vboxLay->addItem( new QSpacerItem( 1, 1 ) );

}

void PrefsDialog::docTab()
{
  QLabel *label;
  QFrame *topFrame = addPage( i18n( "Appearance" ),
                              i18n( "How Kraft starts up." ),
                              DesktopIcon( "queue" ) );

  QVBoxLayout *vboxLay = new QVBoxLayout( topFrame );
  QGridLayout *topLayout = new QGridLayout( topFrame );
  vboxLay->addLayout( topLayout );

  topLayout->setSpacing( spacingHint() );
  topLayout->setColSpacing( 0, spacingHint() );

  label = new QLabel(i18n("Default document type on creation:"), topFrame );
  topLayout->addWidget(label, 0,0);

  mCbDocTypes = new QComboBox( topFrame );
  topLayout->addWidget( mCbDocTypes, 0, 1 );
  mCbDocTypes->insertStringList( DocType::allLocalised() );

  // Localisation on document level
  mCbDocLocale = new QCheckBox( i18n( "enable &localisation on document level" ), topFrame );
  vboxLay->addWidget( mCbDocLocale );

  vboxLay->addWidget( new QWidget( topFrame ) );
}

void PrefsDialog::doctypeTab()
{
  QFrame *topFrame = addPage( i18n( "Document Types" ),
                              i18n( "Edit Details of Document Types." ),
                              DesktopIcon( "queue" ) );

  QVBoxLayout *vboxLay = new QVBoxLayout( topFrame );
  vboxLay->setSpacing( spacingHint() );
  // vboxLay->setColSpacing( 0, spacingHint() );

  mDocTypeEdit = new DocTypeEdit( topFrame );
  vboxLay->addWidget( mDocTypeEdit );
}


void PrefsDialog::slotTextChanged( const QString& )
{
  bool en = false;
  if ( !m_leName->text().isEmpty() ) {
    en = true;
  }

  m_pbCheck->setEnabled( en );
}

void PrefsDialog::readConfig()
{
    m_leHost->setText( KatalogSettings::dbServerName() );
    m_leName->setText( KatalogSettings::dbFile() );
    m_leUser->setText( KatalogSettings::dbUser() );
    m_lePasswd->setText( KatalogSettings::dbPassword() );

    mCbDocLocale->setChecked( KraftSettings::showDocumentLocale() );

    QString t = KraftSettings::doctype();
    if ( t.isEmpty() ) t = DefaultProvider::self()->docType();

    mCbDocTypes->setCurrentText( t );
}

void PrefsDialog::writeConfig()
{
    KatalogSettings::setDbServerName(m_leHost->text());
    KatalogSettings::setDbFile(m_leName->text());
    KatalogSettings::setDbUser(m_leUser->text());
    KatalogSettings::setDbPassword( m_lePasswd->text());
    KatalogSettings::writeConfig();

    KraftSettings::setShowDocumentLocale( mCbDocLocale->isChecked() );
    KraftSettings::setDoctype( mCbDocTypes->currentText() );
    KraftSettings::writeConfig();
}

PrefsDialog::~PrefsDialog()
{
}

void PrefsDialog::slotCheckConnect()
{
  kdDebug() << "Trying database connect to db " << m_leName->text() << endl;

  int x = KraftDB::self()->checkConnect( m_leHost->text(), m_leName->text(),
                                         m_leUser->text(), m_lePasswd->text() );
  kdDebug() << "Connection result: " << x << endl;
  if ( x == 0 ) {
    m_statusLabel->setText( i18n( "Good!" ) );
  } else {
    m_statusLabel->setText( i18n( "Failed" ) );
  }
}

void PrefsDialog::slotOk()
{
  mDocTypeEdit->saveDocTypes();
  writeConfig();
  accept();
}

#include "prefsdialog.moc"
