// Commit: 84c47bbb133304d7ef35642fa1fbb17619d4a43d
/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QSGSTATEOPERATIONS_H
#define QSGSTATEOPERATIONS_H

#include "qsgitem.h"
#include "qsganchors_p.h"

#include <private/qdeclarativestate_p.h>

#include <QtDeclarative/qdeclarativescriptstring.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)

class QSGParentChangePrivate;
class Q_AUTOTEST_EXPORT QSGParentChange : public QDeclarativeStateOperation, public QDeclarativeActionEvent
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSGParentChange)

    Q_PROPERTY(QSGItem *target READ object WRITE setObject)
    Q_PROPERTY(QSGItem *parent READ parent WRITE setParent)
    Q_PROPERTY(QDeclarativeScriptString x READ x WRITE setX)
    Q_PROPERTY(QDeclarativeScriptString y READ y WRITE setY)
    Q_PROPERTY(QDeclarativeScriptString width READ width WRITE setWidth)
    Q_PROPERTY(QDeclarativeScriptString height READ height WRITE setHeight)
    Q_PROPERTY(QDeclarativeScriptString scale READ scale WRITE setScale)
    Q_PROPERTY(QDeclarativeScriptString rotation READ rotation WRITE setRotation)
public:
    QSGParentChange(QObject *parent=0);
    ~QSGParentChange();

    QSGItem *object() const;
    void setObject(QSGItem *);

    QSGItem *parent() const;
    void setParent(QSGItem *);

    QSGItem *originalParent() const;

    QDeclarativeScriptString x() const;
    void setX(QDeclarativeScriptString x);
    bool xIsSet() const;

    QDeclarativeScriptString y() const;
    void setY(QDeclarativeScriptString y);
    bool yIsSet() const;

    QDeclarativeScriptString width() const;
    void setWidth(QDeclarativeScriptString width);
    bool widthIsSet() const;

    QDeclarativeScriptString height() const;
    void setHeight(QDeclarativeScriptString height);
    bool heightIsSet() const;

    QDeclarativeScriptString scale() const;
    void setScale(QDeclarativeScriptString scale);
    bool scaleIsSet() const;

    QDeclarativeScriptString rotation() const;
    void setRotation(QDeclarativeScriptString rotation);
    bool rotationIsSet() const;

    virtual ActionList actions();

    virtual void saveOriginals();
    //virtual void copyOriginals(QDeclarativeActionEvent*);
    virtual void execute(Reason reason = ActualChange);
    virtual bool isReversable();
    virtual void reverse(Reason reason = ActualChange);
    virtual QString typeName() const;
    virtual bool override(QDeclarativeActionEvent*other);
    virtual void rewind();
    virtual void saveCurrentValues();
};

class QSGAnchorChanges;
class QSGAnchorSetPrivate;
class Q_AUTOTEST_EXPORT QSGAnchorSet : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QDeclarativeScriptString left READ left WRITE setLeft RESET resetLeft)
    Q_PROPERTY(QDeclarativeScriptString right READ right WRITE setRight RESET resetRight)
    Q_PROPERTY(QDeclarativeScriptString horizontalCenter READ horizontalCenter WRITE setHorizontalCenter RESET resetHorizontalCenter)
    Q_PROPERTY(QDeclarativeScriptString top READ top WRITE setTop RESET resetTop)
    Q_PROPERTY(QDeclarativeScriptString bottom READ bottom WRITE setBottom RESET resetBottom)
    Q_PROPERTY(QDeclarativeScriptString verticalCenter READ verticalCenter WRITE setVerticalCenter RESET resetVerticalCenter)
    Q_PROPERTY(QDeclarativeScriptString baseline READ baseline WRITE setBaseline RESET resetBaseline)
    //Q_PROPERTY(QSGItem *fill READ fill WRITE setFill RESET resetFill)
    //Q_PROPERTY(QSGItem *centerIn READ centerIn WRITE setCenterIn RESET resetCenterIn)

    /*Q_PROPERTY(qreal margins READ margins WRITE setMargins NOTIFY marginsChanged)
    Q_PROPERTY(qreal leftMargin READ leftMargin WRITE setLeftMargin NOTIFY leftMarginChanged)
    Q_PROPERTY(qreal rightMargin READ rightMargin WRITE setRightMargin NOTIFY rightMarginChanged)
    Q_PROPERTY(qreal horizontalCenterOffset READ horizontalCenterOffset WRITE setHorizontalCenterOffset NOTIFY horizontalCenterOffsetChanged())
    Q_PROPERTY(qreal topMargin READ topMargin WRITE setTopMargin NOTIFY topMarginChanged)
    Q_PROPERTY(qreal bottomMargin READ bottomMargin WRITE setBottomMargin NOTIFY bottomMarginChanged)
    Q_PROPERTY(qreal verticalCenterOffset READ verticalCenterOffset WRITE setVerticalCenterOffset NOTIFY verticalCenterOffsetChanged())
    Q_PROPERTY(qreal baselineOffset READ baselineOffset WRITE setBaselineOffset NOTIFY baselineOffsetChanged())*/

public:
    QSGAnchorSet(QObject *parent=0);
    virtual ~QSGAnchorSet();

    QDeclarativeScriptString left() const;
    void setLeft(const QDeclarativeScriptString &edge);
    void resetLeft();

    QDeclarativeScriptString right() const;
    void setRight(const QDeclarativeScriptString &edge);
    void resetRight();

    QDeclarativeScriptString horizontalCenter() const;
    void setHorizontalCenter(const QDeclarativeScriptString &edge);
    void resetHorizontalCenter();

    QDeclarativeScriptString top() const;
    void setTop(const QDeclarativeScriptString &edge);
    void resetTop();

    QDeclarativeScriptString bottom() const;
    void setBottom(const QDeclarativeScriptString &edge);
    void resetBottom();

    QDeclarativeScriptString verticalCenter() const;
    void setVerticalCenter(const QDeclarativeScriptString &edge);
    void resetVerticalCenter();

    QDeclarativeScriptString baseline() const;
    void setBaseline(const QDeclarativeScriptString &edge);
    void resetBaseline();

    QSGItem *fill() const;
    void setFill(QSGItem *);
    void resetFill();

    QSGItem *centerIn() const;
    void setCenterIn(QSGItem *);
    void resetCenterIn();

    /*qreal leftMargin() const;
    void setLeftMargin(qreal);

    qreal rightMargin() const;
    void setRightMargin(qreal);

    qreal horizontalCenterOffset() const;
    void setHorizontalCenterOffset(qreal);

    qreal topMargin() const;
    void setTopMargin(qreal);

    qreal bottomMargin() const;
    void setBottomMargin(qreal);

    qreal margins() const;
    void setMargins(qreal);

    qreal verticalCenterOffset() const;
    void setVerticalCenterOffset(qreal);

    qreal baselineOffset() const;
    void setBaselineOffset(qreal);*/

    QSGAnchors::Anchors usedAnchors() const;

/*Q_SIGNALS:
    void leftMarginChanged();
    void rightMarginChanged();
    void topMarginChanged();
    void bottomMarginChanged();
    void marginsChanged();
    void verticalCenterOffsetChanged();
    void horizontalCenterOffsetChanged();
    void baselineOffsetChanged();*/

private:
    friend class QSGAnchorChanges;
    Q_DISABLE_COPY(QSGAnchorSet)
    Q_DECLARE_PRIVATE(QSGAnchorSet)
};

class QSGAnchorChangesPrivate;
class Q_AUTOTEST_EXPORT QSGAnchorChanges : public QDeclarativeStateOperation, public QDeclarativeActionEvent
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSGAnchorChanges)

    Q_PROPERTY(QSGItem *target READ object WRITE setObject)
    Q_PROPERTY(QSGAnchorSet *anchors READ anchors CONSTANT)

public:
    QSGAnchorChanges(QObject *parent=0);
    ~QSGAnchorChanges();

    virtual ActionList actions();

    QSGAnchorSet *anchors();

    QSGItem *object() const;
    void setObject(QSGItem *);

    virtual void execute(Reason reason = ActualChange);
    virtual bool isReversable();
    virtual void reverse(Reason reason = ActualChange);
    virtual QString typeName() const;
    virtual bool override(QDeclarativeActionEvent*other);
    virtual bool changesBindings();
    virtual void saveOriginals();
    virtual bool needsCopy() { return true; }
    virtual void copyOriginals(QDeclarativeActionEvent*);
    virtual void clearBindings();
    virtual void rewind();
    virtual void saveCurrentValues();

    QList<QDeclarativeAction> additionalActions();
    virtual void saveTargetValues();
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QSGParentChange)
QML_DECLARE_TYPE(QSGAnchorSet)
QML_DECLARE_TYPE(QSGAnchorChanges)

QT_END_HEADER

#endif // QSGSTATEOPERATIONS_H

