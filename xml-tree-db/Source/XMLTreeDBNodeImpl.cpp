/*
    Copyright (c) 2019-2020 Xavier Leclercq
    Released under the MIT License
    See https://github.com/DiplodocusDB/TreeDB/blob/master/LICENSE.txt
*/

#include "XMLTreeDBNodeImpl.h"
#include "XMLTreeDBImpl.h"
#include "DiplodocusDB/TreeDB/Core/TreeDBErrorCategory.h"

namespace DiplodocusDB
{

XMLTreeDBNodeImpl::XMLTreeDBNodeImpl(XMLTreeDBNodeImpl* parent, pugi::xml_node node)
    : TreeDBNodeImpl(node.name()), m_parent(parent), m_node(node)
{
}

XMLTreeDBNodeImpl::~XMLTreeDBNodeImpl()
{
}

bool XMLTreeDBNodeImpl::isRoot() const
{
    return (m_parent == nullptr);
}

TreeDBNode XMLTreeDBNodeImpl::parent(Ishiko::Error& error)
{
    TreeDBNode result;
    if (m_parent)
    {
        result = TreeDBNode(m_parent->shared_from_this());
    }
    return result;
}

std::vector<TreeDBNode> XMLTreeDBNodeImpl::childNodes(Ishiko::Error& error)
{
    std::vector<TreeDBNode> result;
    loadChildren(error);
    for (std::shared_ptr<XMLTreeDBNodeImpl>& child : m_children)
    {
        result.push_back(TreeDBNode(child));
    }
    return result;
}

TreeDBNode XMLTreeDBNodeImpl::child(const std::string& name, Ishiko::Error& error)
{
    TreeDBNode result;

    loadChildren(error);
    for (std::shared_ptr<XMLTreeDBNodeImpl>& child : m_children)
    {
        if (child->name() == name)
        {
            result = TreeDBNode(child);
            break;
        }
    }

    return result;
}

TreeDBNode XMLTreeDBNodeImpl::previousSibling(Ishiko::Error& error)
{
    TreeDBNode result;
    return result;
}

TreeDBNode XMLTreeDBNodeImpl::previousSibling(const std::string& name, Ishiko::Error& error)
{
    TreeDBNode result;
    return result;
}

TreeDBNode XMLTreeDBNodeImpl::nextSibling(Ishiko::Error& error)
{
    TreeDBNode result;

    if (m_parent)
    {
        for (size_t i = 0; i < m_parent->m_children.size(); ++i)
        {
            if (m_parent->m_children[i].get() == this)
            {
                ++i;
                if (i < m_parent->m_children.size())
                {
                    result = TreeDBNode(m_parent->m_children[i]);
                }
                break;
            }
        }
    }

    return result;
}

TreeDBNode XMLTreeDBNodeImpl::nextSibling(const std::string& name, Ishiko::Error& error)
{
    // TODO
    TreeDBNode result;
    return result;
}

TreeDBNode XMLTreeDBNodeImpl::insertChildNode(size_t index, const std::string& name, const TreeDBValue& value,
    Ishiko::Error& error)
{
    pugi::xml_node newNode = m_node.append_child(name.c_str());
    std::shared_ptr<XMLTreeDBNodeImpl> newNodeImpl = std::make_shared<XMLTreeDBNodeImpl>(this, newNode);
    newNodeImpl->value() = value;
    m_children.push_back(newNodeImpl);
    return TreeDBNode(newNodeImpl);
}

TreeDBNode XMLTreeDBNodeImpl::insertChildNodeBefore(const TreeDBNode& nextChild, const std::string& name,
    const TreeDBValue& value, Ishiko::Error& error)
{
    // TODO
    TreeDBNode result;
    return result;
}

TreeDBNode XMLTreeDBNodeImpl::insertChildNodeAfter(const TreeDBNode& previousChild, const std::string& name,
    const TreeDBValue& value, Ishiko::Error& error)
{
    // TODO
    TreeDBNode result;
    return result;
}

TreeDBNode XMLTreeDBNodeImpl::appendChildNode(const std::string& name, const TreeDBValue& value, Ishiko::Error& error)
{
    pugi::xml_node newNode = m_node.append_child(name.c_str());
    std::shared_ptr<XMLTreeDBNodeImpl> newNodeImpl = std::make_shared<XMLTreeDBNodeImpl>(this, newNode);
    newNodeImpl->value() = value;
    m_children.push_back(newNodeImpl);
    return TreeDBNode(newNodeImpl);
}

TreeDBNode XMLTreeDBNodeImpl::setChildNode(const std::string& name, const TreeDBValue& value, Ishiko::Error& error)
{
    TreeDBNode result;

    loadChildren(error);
    for (size_t i = 0; i < m_children.size(); ++i)
    {
        if (m_children[i]->name() == name)
        {
            result = TreeDBNode(m_children[i]);
            m_children[i]->value() = value;
            break;
        }
    }
    if (!result)
    {
        pugi::xml_node newNode = m_node.append_child(name.c_str());
        std::shared_ptr<XMLTreeDBNodeImpl> newNodeImpl = std::make_shared<XMLTreeDBNodeImpl>(this, newNode);
        newNodeImpl->value() = value;
        m_children.push_back(newNodeImpl);
        result = TreeDBNode(newNodeImpl);
    }

    return result;
}

size_t XMLTreeDBNodeImpl::removeChildNode(const std::string& name, Ishiko::Error& error)
{
    return false;
}

size_t XMLTreeDBNodeImpl::removeAllChildNodes(Ishiko::Error& error)
{
    loadChildren(error);
    for (std::shared_ptr<XMLTreeDBNodeImpl>& child : m_children)
    {
        m_node.remove_child(child->m_node);
    }

    size_t result = m_children.size();
    m_children.clear();
    return result;
}

void XMLTreeDBNodeImpl::updateValue()
{
    // Remove any current data
    pugi::xml_node pcdataNode = m_node.first_child();
    if (pcdataNode.type() == pugi::node_pcdata)
    {
        m_node.remove_child(pcdataNode);
    }
    pugi::xml_node valueNode = m_node.child("data");
    if (valueNode)
    {
        m_node.remove_child(valueNode);
    }

    // Set the new data
    const TreeDBValue& v = value();
    if (v.type() == EPrimitiveDataType::eNULL)
    {
        // The data type of a node is null by default. If we didn't do that
        // then all the intermediate nodes without any value would have a
        // data-type attribute which would be cumbersome to write manually.

        m_node.remove_attribute("data-type");
    }
    else if (v.type() != EPrimitiveDataType::eNULL)
    {
        pugi::xml_attribute attributeNode = m_node.attribute("data-type");
        if (attributeNode)
        {
            attributeNode.set_value("utf8string");
        }
        else
        {
            m_node.append_attribute("data-type").set_value("utf8string");
        }
        if (m_children.empty())
        {
            pugi::xml_node pcdataNode = m_node.first_child();
            if (!pcdataNode || (pcdataNode.type() != pugi::node_pcdata))
            {
                pcdataNode = m_node.prepend_child(pugi::node_pcdata);
            }
            pcdataNode.set_value(v.asUTF8String().c_str());
        }
        else
        {
            pugi::xml_node valueNode = m_node.prepend_child("data");
            valueNode.append_child(pugi::node_pcdata).set_value(v.asUTF8String().c_str());
        }
    }
    for (size_t i = 0; i < m_children.size(); ++i)
    {
        m_children[i]->updateValue();
    }
}

void XMLTreeDBNodeImpl::loadChildren(Ishiko::Error& error)
{
    if (m_children.empty())
    {
        pugi::xml_node childNode = m_node.first_child();
        while (childNode)
        {
            std::shared_ptr<XMLTreeDBNodeImpl> newNode = std::make_shared<XMLTreeDBNodeImpl>(this, childNode);
            pugi::xml_attribute dataTypeAttribute = childNode.attribute("data-type");
            if (dataTypeAttribute)
            {
                if (strcmp(dataTypeAttribute.as_string(), "null") == 0)
                {
                    m_children.push_back(newNode);
                }
                else if (strcmp(dataTypeAttribute.as_string(), "utf8string") == 0)
                {
                    newNode->value().setUTF8String(childNode.child_value());
                    m_children.push_back(newNode);
                }
                else
                {
                    Fail(error, TreeDBErrorCategory::eGeneric, "Unknown data type encountered while loading child node", __FILE__, __LINE__);
                }
            }
            else
            {
                // No data-type attribute is equivalent to null
                m_children.push_back(newNode);
            }
            childNode = childNode.next_sibling();
        }
    }
}

}
