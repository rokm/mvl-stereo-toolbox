/*
 * File Image Pair Source: source
 * Copyright (C) 2013 Rok Mandeljc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#ifndef SOURCE_FILE_H
#define SOURCE_FILE_H

#include <QtNetwork>

#include "ImagePairSource.h"


class SourceFile : public ImagePairSource
{
    Q_OBJECT

public:
    SourceFile (QObject * = 0);
    virtual ~SourceFile ();

    virtual void stopSource ();

    virtual QWidget *createConfigWidget (QWidget * = 0);

    void loadImagePair (const QString &, const QString &, bool);

    const QString &getLeftFilename () const;
    int getLeftWidth () const;
    int getLeftHeight () const;
    int getLeftChannels () const;

    const QString &getRightFilename () const;
    int getRightWidth () const;
    int getRightHeight () const;
    int getRightChannels () const;

    bool getPeriodicRefreshState () const;
    int getRefreshPeriod () const;
    
public slots:
    void setPeriodicRefreshState (bool);
    void setRefreshPeriod (int);

protected slots:
    void periodicRefresh ();
    void processRemoteReply (QNetworkReply *);

protected:
    void loadLocalImages ();
    void loadRemoteImages ();
    void imageLoadingError (const QString &);

signals:
    void periodicRefreshStateChanged (bool);
    void refreshPeriodChanged (int);
    
protected:
    QTimer *refreshTimer;
    int refreshPeriod;

    QString filenameLeft;
    QString filenameRight;

    bool remote;
    QNetworkAccessManager *network;
    QNetworkReply *replyLeft, *replyRight;
};

#endif
