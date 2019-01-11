/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "triangleelement.h"

#include <QColor>
#include <QRectF>
#include <QSGGeometryNode>
#include <QSGNode>

namespace QMLExtensions {
    TriangleElement::TriangleElement(QQuickItem *parent) :
        QQuickItem(parent),
        m_Geometry(QSGGeometry::defaultAttributes_Point2D(), 3),
        m_IsFlipped(false),
        m_IsVertical(false)
    {
        setFlag(ItemHasContents);
        m_Material.setColor(m_Color);
    }

    QSGNode *TriangleElement::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) {
        QSGGeometryNode *node = 0;

        if ((width() <= 0 || height() <= 0) && (oldNode != nullptr)) {
            delete oldNode;
            return nullptr;
        }

        if (oldNode == nullptr) {
            node = new QSGGeometryNode();
            node->setGeometry(&m_Geometry);
            node->setMaterial(&m_Material);
        } else {
            node = static_cast<QSGGeometryNode *>(oldNode);
        }

        updateTriangle(&m_Geometry);
        node->markDirty(QSGNode::DirtyGeometry);
        node->markDirty(QSGNode::DirtyMaterial);

        return node;
    }

    void TriangleElement::updateTriangle(QSGGeometry *geometry) {
        QSGGeometry::Point2D *v = geometry->vertexDataAsPoint2D();
        const QRectF rect = boundingRect();

        if (m_IsVertical) {
            if (m_IsFlipped) {
                v[0].x = rect.right();
                v[0].y = rect.top();
                v[1].x = rect.right();
                v[1].y = rect.bottom();
                v[2].x = rect.left();
                v[2].y = rect.top() + rect.height()/2.0;
            } else {
                v[0].x = rect.left();
                v[0].y = rect.top();
                v[1].x = rect.right();
                v[1].y = rect.top() + rect.height()/2.0;
                v[2].x = rect.left();
                v[2].y = rect.bottom();
            }
        } else { // horizontal
            if (m_IsFlipped) {
                v[0].x = rect.left();
                v[0].y = rect.top();
                v[1].x = rect.right();
                v[1].y = rect.top();
                v[2].x = rect.left() + rect.width()/2.0;
                v[2].y = rect.bottom();
            } else {
                v[0].x = rect.left();
                v[0].y = rect.bottom();
                v[1].x = rect.left() + rect.width()/2.0;
                v[1].y = rect.top();
                v[2].x = rect.right();
                v[2].y = rect.bottom();
            }
        }
    }

    void TriangleElement::setColor(const QColor &color) {
        if (m_Color != color) {
            m_Color = color;
            m_Material.setColor(m_Color);
            update();
            emit colorChanged(color);
        }
    }

    void TriangleElement::setIsFlipped(bool value) {
        if (m_IsFlipped != value) {
            m_IsFlipped = value;
            emit isFlippedChanged(value);
            update();
        }
    }

    void TriangleElement::setIsVertical(bool value) {
        if (m_IsVertical != value) {
            m_IsVertical = value;
            emit isVerticalChanged(value);
            update();
        }
    }
}
