#include "activities/dataviewer/panels/Node.hpp"

#include <utility>

#include <QList>
#include <QString>
#include <QtGlobal>

Node::Node(QString name, int node_id, Node* parent) : m_name(std::move(name)), m_id(node_id), m_parent(parent) {}

Node::~Node() {
    for (Node* child : m_children) {
        delete child;
    }
    m_children.clear();
}

QString Node::name() const {
    return m_name;
}

QString Node::value() const {
    return m_value;
}

int Node::id() const {
    return m_id;
}

Node* Node::parent() const {
    return m_parent;
}

void Node::addChild(Node* child) {
    m_children.append(child);
}

QList<Node*> Node::children() const {
    return m_children;
}
