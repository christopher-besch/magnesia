#ifndef NODE_HPP
#define NODE_HPP

#include <QList>
#include <QString>

class Node {
    Q_DISABLE_COPY_MOVE(Node)
  public:
    Node(QString name, int node_id, Node* parent = nullptr);
    ~Node();

    [[nodiscard]] QString      name() const;
    [[nodiscard]] QString      value() const;
    [[nodiscard]] int          id() const;
    [[nodiscard]] Node*        parent() const;
    void                       addChild(Node* child);
    [[nodiscard]] QList<Node*> children() const;

  private:
    QString      m_name;
    QString      m_value;
    int          m_id;
    Node*        m_parent;
    QList<Node*> m_children;
};

#endif // NODE_HPP
