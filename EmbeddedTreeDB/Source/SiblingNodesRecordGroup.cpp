/*
    Copyright (c) 2019 Xavier Leclercq

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/

#include "SiblingNodesRecordGroup.h"
#include "Record.h"

namespace DiplodocusDB
{

SiblingNodesRecordGroup::SiblingNodesRecordGroup(const EmbeddedTreeDBNodeImpl& node)
{
    m_siblings.emplace_back(node);
}

void SiblingNodesRecordGroup::write(PageRepositoryWriter& writer, Ishiko::Error& error) const
{
    Record nodeStartRecord(Record::ERecordType::eSiblingNodesStart);
    nodeStartRecord.write(writer, error);
    if (error)
    {
        return;
    }

    for (const EmbeddedTreeDBNodeImpl& node : m_siblings)
    {
        writeNode(writer, node, error);
        if (error)
        {
            return;
        }
    }

    Record nodeEndRecord(Record::ERecordType::eSiblingNodesEnd);
    nodeEndRecord.write(writer, error);
}

void SiblingNodesRecordGroup::writeNode(PageRepositoryWriter& writer, const EmbeddedTreeDBNodeImpl& node,
    Ishiko::Error& error)
{

    if (node.isRoot())
    {
        Record nameRecord(node.name());
        nameRecord.write(writer, error);
        if (error)
        {
            return;
        }
    }
    else
    {
        Record parentRecord(Record::ERecordType::eParentNodeID, node.parentNodeID());
        parentRecord.write(writer, error);
        if (error)
        {
            return;
        }

        Record nameRecord(node.name());
        nameRecord.write(writer, error);
        if (error)
        {
            return;
        }

        if (!node.nodeID().isNull())
        {
            Record idRecord(Record::ERecordType::eNodeID, node.nodeID());
            idRecord.write(writer, error);
            if (error)
            {
                return;
            }
        }
    }

    if (node.value().type() != DataType(EPrimitiveDataType::eNULL))
    {
        Record record(Record::ERecordType::eInlineValue, node.value());
        record.write(writer, error);
        if (error)
        {
            return;
        }
    }
}

}