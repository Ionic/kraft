/***************************************************************************
                              portalview.h
                           -------------------
    begin                : 2004-05-09
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

#ifndef _PORTALVIEW_H
#define _PORTALVIEW_H

#include <QUrl>
#include <QStackedWidget>
#include <QListWidget>

// include files
#include <kpagewidget.h>

#include "docguardedptr.h"

/**
 *
 */
class QWidget;
class DocDigestView;
class dbID;
class PortalHtmlView;
class ArchDocDigest;

class PortalView : public QWidget
{
  Q_OBJECT

public:
  PortalView (QWidget *parent=0, const char *name=0 );
  ~PortalView();
  DocDigestView* docDigestView() { return mDocDigestView; }
  void systemInitError( const QString& );
  QString ptag( const QString&,  const QString& c = QString() ) const;

public slots:
  void slotBuildView();
  void fillCatalogDetails();
  void fillSystemDetails();

protected slots:
  void slotCreateDocument();

private slots:
  void changePage(QListWidgetItem *current, QListWidgetItem *previous);

signals:
  void openKatalog( const QString& );
  void katalogToXML( const QString& );
  void createDocument();
  void openDocument( const QString& );
  void copyDocument( const QString& );
  void viewDocument( const QString& );
  void openArchivedDocument( const ArchDocDigest& );
  void documentSelected( const QString& );
  void archivedDocSelected( const ArchDocDigest& );
  void printDocument( const QString& );

private:
  QString printKatLine( const QString&, int ) const;
  void createIcons();
  QWidget *katalogDetails();
  QWidget *systemDetails();
  QWidget *documentDigests();

  QString systemViewHeader() const;

  DocDigestView   *mDocDigestView;
  PortalHtmlView  *mCatalogBrowser;
  PortalHtmlView  *mSystemBrowser;

  QListWidget     *_contentsWidget;
  QStackedWidget  *_pagesWidget;

  int mDocDigestIndex;
  int mCatalogIndex;
  int mSystemIndex;
};

#endif

/* END */

