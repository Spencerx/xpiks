#ifndef SELECTEDINDICESMOCK_H
#define SELECTEDINDICESMOCK_H

#include <Artworks/iselectedindicessource.h>
#include <Helpers/indicesranges.h>
#include <initializer_list>

namespace Mocks {
    class SelectedIndicesSourceMock: public Artworks::ISelectedIndicesSource {
    public:
        SelectedIndicesSourceMock(std::vector<int> const &indices):
            m_Indices(indices)
        { }

        SelectedIndicesSourceMock(std::initializer_list<int> indices):
            m_Indices(indices)
        { }

        SelectedIndicesSourceMock(Helpers::IndicesRanges const &ranges):
            m_Indices(ranges.retrieveIndices())
        { }

        // ISelectedIndicesSource interface
    public:
        virtual std::vector<int> getSelectedIndices() override { return m_Indices; }

    public:
        size_t size() const { return m_Indices.size(); }

    private:
        std::vector<int> m_Indices;
    };
}

#endif // SELECTEDINDICESMOCK_H
