/***************************************************************************
                  ArchDoc.cpp  - an archived document.
                             -------------------
    begin                : Mit Dez 31 19:24:05 CET 2003
    copyright            : (C) 2003 by Klaas Freitag
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

#include <QString>
#include <QSqlQuery>
#include <QDateTime>

// include files for KDE
#include <kglobal.h>

#include <klocale.h>
#include <kdebug.h>

// application specific includes
#include "archdoc.h"
#include "documentman.h"
#include "kraftdb.h"
#include "defaultprovider.h"


ArchDoc::ArchDoc()
  :mLocale( "kraft" )
{

}

ArchDoc::ArchDoc( const dbID& id )
  :mLocale( "kraft" )
{
  /* load archive from database */
  loadFromDb( id );
}

ArchDoc::~ArchDoc()
{
}


QString ArchDoc::docIdentifier() const
{
  QString re = docType();

  return i18n("%1 for %2 (Id %3)").arg( docType() ).arg( ident() );
}

Geld ArchDoc::nettoSum()
{
  return positions().sumPrice();
}

Geld ArchDoc::bruttoSum()
{
  Geld g = nettoSum();
  g += taxSum();
  return g;
}

Geld ArchDoc::taxSum()
{
  return positions().taxSum( tax(), reducedTax() ); // DocumentMan::self()->tax( date() ),
  // DocumentMan::self()->reducedTax( date() ) );
}

Geld ArchDoc::fullTaxSum()
{
  return positions().fullTaxSum( tax() );
}

Geld ArchDoc::reducedTaxSum()
{
  return positions().reducedTaxSum( reducedTax() );
}

double ArchDoc::tax()
{
  return mTax;
}

double ArchDoc::reducedTax()
{
  return mReducedTax;
}

void ArchDoc::loadFromDb( dbID id )
{
  mArchDocID = id;

  QSqlQuery q;
  q.prepare("SELECT archDocID, ident, docType, docDescription, clientAddress, clientUid, " // pos 0..5
            "salut, goodbye, printDate, date, pretext, posttext, country, language, " // pos 6..13
            "projectLabel,tax, reducedTax, state from archdoc WHERE archDocID=:id" ); // pos 14..17
  q.bindValue(":id", id.toInt());
  q.exec();

  kDebug() << "Loading document id " << id.toString() << endl;

  if( q.next()) {
    kDebug() << "loading archived document with ident " << id.toString() << endl;
    QString docID;
    QString country;
    QString lang;
    docID         = q.value( 0 ).toString();
    mIdent        = q.value( 1 ).toString();
    mDocType      = q.value( 2 ).toString();
    mAddress      = q.value( 4 ).toString();
    mClientUid    = q.value( 5 ).toString();
    mSalut        = q.value( 6 ).toString();
    mGoodbye      = q.value( 7 ).toString();
    QVariant v    = q.value( 8 );
    mPrintDate    = v.toDateTime();
    mDate         = q.value( 9 ).toDate();
    mPreText      = KraftDB::self()->mysqlEuroDecode( q.value( 10 ).toString() );
    mPostText     = KraftDB::self()->mysqlEuroDecode( q.value( 11 ).toString() );
    country       = q.value( 12 ).toString();
    lang          = q.value( 13 ).toString();
    mProjectLabel = q.value( 14 ).toString();
    mTax          = q.value( 15 ).toDouble();
    mReducedTax   = q.value( 16 ).toDouble();
    mState        = q.value( 17 ).toInt();

    KConfig *cfg = KGlobal::config().data();
    mLocale.setCountry( country, cfg );
    mLocale.setLanguage( lang , cfg );

    loadPositions( docID );
    loadAttributes( docID );
  } else {
    kDebug() << "ERR: Could not load archived doc with id " << id.toString() << endl;
  }
}

void ArchDoc::loadPositions( const QString& archDocId )
{
  mPositions.clear();

  if ( archDocId.isEmpty() /* || ! archDocId.isNum() */ ) {
    kDebug() << "ArchDocId is not crappy: " << archDocId << endl;
    return;
  }

  QSqlQuery q;
  q.prepare("SELECT archPosID, archDocID, ordNumber, kind, postype, text, amount, " // pos 0..6
            "unit, price, overallPrice, taxType FROM archdocpos WHERE archDocID=:id ORDER BY ordNumber"); // pos 7..10
  q.bindValue("id", archDocId);
  q.exec();

  while( q.next() ) {
    ArchDocPosition pos;
    pos.mPosNo = q.value( 2 ).toString();
    pos.mKind = q.value( 3 ).toString();
    pos.mText = q.value( 5 ).toString();
    pos.mAmount = q.value( 6 ).toDouble();
    pos.mUnit  = q.value( 7 ).toString();
    pos.mUnitPrice = Geld( q.value( 8 ).toDouble() );
    pos.mOverallPrice = q.value( 9 ).toDouble();

    int tt = q.value( 10 ).toInt();
    if ( tt == 1 )
      pos.mTaxType = DocPositionBase::TaxNone;
    else if ( tt == 2 )
      pos.mTaxType = DocPositionBase::TaxReduced;
    else if ( tt == 3 )
      pos.mTaxType = DocPositionBase::TaxFull;

    mPositions.append( pos );
  }
}

void ArchDoc::loadAttributes( const QString& archDocId )
{
  mAttribs.clear();

  if ( archDocId.isEmpty() ) {
    kDebug() << "ArchDocId is Empty!" << endl;
    return;
  }

  QSqlQuery q;
  q.prepare("SELECT name, value FROM archPosAttribs WHERE archDocID=:id");
  q.bindValue(":id", archDocId);
  q.exec();

  while ( q.next() ) {
    QString name  = q.value( 0 ).toString();
    QString value = q.value( 1 ).toString();

    if ( !name.isEmpty() ) {
      mAttribs[ name ] = value;
    } else {
      kDebug() << "Empty attribute name in archive!"  << endl;
    }
  }
}

ArchDocDigest ArchDoc::toDigest()
{
    return ArchDocDigest(mPrintDate, mState, mIdent, mArchDocID);
}

/* ###################################################################### */

ArchDocDigest::ArchDocDigest()
{

}

ArchDocDigest::ArchDocDigest( QDateTime dt,  int s, const QString& ident, dbID id )
  : mPrintDate( dt ),
    mState( s ),
    mArchDocId( id ),
    mIdent( ident )
{

}

ArchDocDigest::~ArchDocDigest()
{

}

QString ArchDocDigest::printDateString() const
{
  return DefaultProvider::self()->locale()->formatDateTime( mPrintDate, KLocale::ShortDate );
}

/* ###################################################################### */


