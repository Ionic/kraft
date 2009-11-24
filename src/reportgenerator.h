/***************************************************************************
                    reportgenerator.h - report generation
                             -------------------
    begin                : July 2006
    copyright            : (C) 2006 by Klaas Freitag
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
#ifndef REPORTGENERATOR_H
#define REPORTGENERATOR_H

#include <QFile>
#include <QObject>
#include <QProcess>
#include <QTextStream>

#include "kraftdoc.h"

class dbID;
class KProcess;
class QFile;


class ReportGenerator : public QObject
{
  Q_OBJECT

public:
  ~ReportGenerator();

  static ReportGenerator *self();

  void runTrml2Pdf( const QString&, const QString&, const QString& );
  QString findTrml2Pdf();

signals:
  void pdfAvailable( const QString& filename );

public slots:
  void createPdfFromArchive( const QString&, dbID );

protected slots:
  void trml2pdfFinished( int );
  void slotReceivedStdout();
  void slotReceivedStderr();
  void slotError( QProcess::ProcessError );
  QString findTemplate( const QString& );
  
private:
  QString fillupTemplateFromArchive( const dbID& );

  QString registerDictionary( const QString&, const QString& ) const;
  QString registerTag( const QString&, const QString& ) const;
  QString registerDictTag( const QString&, const QString&, const QString& ) const;

  
  QString escapeTrml2pdfXML( const QString& str ) const;

  QString rmlString( const QString& str, const QString& paraStyle = QString() ) const;

  ReportGenerator();
  QString mOutFile;
  QString mErrors;
  QString mMergeIdent;
  bool    mHaveMerge;
  QString mWatermarkFile;

  QFile mFile;
  QTextStream mTargetStream;

  static ReportGenerator *mSelf;
  static KProcess *mProcess;
};

#endif
