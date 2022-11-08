/**
 * @file ecv.cpp
 * @brief Implementation of \a ecv.hpp
 * @author lhm
 */

// Project's headers
#include <ecv.hpp>

namespace ecv {

namespace {
struct Column;

/*****************************************************************************/
template<typename T>
struct IDataObj
{
    virtual ~IDataObj() noexcept = default;

    virtual void remove(void) noexcept = 0;
    virtual void restore(void) noexcept = 0;
    virtual void erase(void) noexcept = 0;

    T *_l{ nullptr }, *_r{ nullptr }, *_u{ nullptr }, *_d{ nullptr };
};

/*****************************************************************************/
struct Node : public IDataObj<Node>
{
    virtual void remove(void) noexcept override;
    virtual void restore(void) noexcept override;
    virtual void erase(void) noexcept override;

    int     _row{ -1 };
    Column* _col{ nullptr }; // Head
};

/*****************************************************************************/
struct Column : public IDataObj<Column>
{
    virtual void remove(void) noexcept override;
    virtual void restore(void) noexcept override;
    virtual void erase(void) noexcept override;

public:
    Node _head; // Head
    int  _size; // Number of ones in the column, used for branching optimization
};

/*****************************************************************************/
void
Node::remove(void) noexcept
{
    _u->_d = _d;
    _d->_u = _u;
    --_col->_size;
}

/*****************************************************************************/
void
Node::restore(void) noexcept
{
    _u->_d = this;
    _d->_u = this;
    ++_col->_size;
}

/*****************************************************************************/
void
Node::erase(void) noexcept
{
    remove();

    _l->_r = _r;
    _r->_l = _l;
}

/*****************************************************************************/
void
Column::remove(void) noexcept
{
    _l->_r = _r;
    _r->_l = _l;

    for (auto ccell{ _head._d }; ccell != &_head; ccell = ccell->_d)
        for (auto rcell{ ccell->_r }; rcell != ccell; rcell = rcell->_r)
            rcell->remove();
}

/*****************************************************************************/
void
Column::restore(void) noexcept
{
    for (auto ccell{ _head._u }; ccell != &_head; ccell = ccell->_u)
        for (auto rcell{ ccell->_l }; rcell != ccell; rcell = rcell->_l)
            rcell->restore();

    _l->_r = this;
    _r->_l = this;
}

/*****************************************************************************/
void
Column::erase(void) noexcept
{
    remove();

    _l->_r = _r;
    _r->_l = _l;
}

} // anonymous

struct DLX::Impl
{
    Column              _head;
    std::vector<Column> _cols;
    std::vector<Node>   _nodes;

    std::vector<Solution> _solutions;
    std::vector<int>      _curSol;

    Column*               col_select(void) noexcept;
    [[maybe_unused]] bool init(const std::vector<bool>& data,
                               size_t                   R,
                               size_t                   C,
                               const std::vector<int>&  rowsList) noexcept;
    std::vector<Solution> solve(uint32_t) noexcept;
    bool                  _solve(const uint32_t&, uint32_t&) noexcept;

    auto zeros(void) noexcept { return (_head._r == &_head) && (_head._l == &_head); }
};

/*****************************************************************************/
Column*
DLX::Impl::col_select(void) noexcept
{
    auto ret{ _head._r };
    for (auto cdt{ ret->_r }; &_head != cdt; cdt = cdt->_r) {
        if (cdt->_size < ret->_size)
            ret = cdt;
    }

    return ret;
}

/*****************************************************************************/
bool
DLX::Impl::init(const std::vector<bool>& data,
                size_t                   R,
                size_t                   C,
                const std::vector<int>&  rowsList) noexcept
{
    if (0 == R || 0 == C || std::size(data) != R * C)
        return false;

    _curSol.reserve(std::size(rowsList));
    _cols.resize(C);
    _nodes.resize(R * C);

    _head._r = &_cols[0];
    _head._l = &_cols[C - 1];
    _cols[0]._l = &_head;
    _cols[C - 1]._r = &_head;

    for (size_t i{ 0 }; i < C - 1; ++i) {
        _cols[i]._r = &_cols[i + 1];
        _cols[i + 1]._l = &_cols[i];
    }

    for (size_t i{ 0 }; i < C; ++i)
        _cols[i]._size = R;

    for (size_t i{ 0 }, k{ 0 }; i < R; ++i) {
        for (size_t j{ 0 }; j < C; ++j, ++k) {
            if (k < C)
                _cols[j]._head._d = &_nodes[k];
            if (k >= (R - 1) * C)
                _cols[j]._head._u = &_nodes[k];

            _nodes[k]._row = rowsList[i];
            _nodes[k]._col = &_cols[j];

            _nodes[k]._l = (0 == k % C) ? &_nodes[k + C - 1] : &_nodes[k - 1];
            _nodes[k]._r = (C - 1 == k % C) ? &_nodes[k - C + 1] : &_nodes[k + 1];
            _nodes[k]._u = (k < C) ? &_cols[j]._head : &_nodes[k - C];
            _nodes[k]._d = (k >= (R - 1) * C) ? &_cols[j]._head : &_nodes[k + C];
        }
    }

    for (size_t i{ 0 }, k{ 0 }; i < R; ++i)
        for (size_t j{ 0 }; j < C; ++j, ++k)
            if (!data[k])
                _nodes[k].erase();

    return true;
}

/*****************************************************************************/
std::vector<DLX::Solution>
DLX::Impl::solve(uint32_t max_solutions = std::numeric_limits<uint32_t>::max()) noexcept
{
    uint32_t sol_count{ 0 };
    _solutions.clear();

    if (!std::empty(_nodes))
        _solve(max_solutions, sol_count);
    return _solutions;
}

/*****************************************************************************/
DLX::DLX(const std::vector<bool>& data,
         size_t                   rows,
         size_t                   cols,
         const std::vector<int>&  rowsList) noexcept
  : pimpl{ std::make_shared<Impl>() }
{
    pimpl->init(data, rows, cols, rowsList);
}

/*****************************************************************************/
bool
DLX::Impl::_solve(const uint32_t& max_solutions, uint32_t& sol_count) noexcept
{
    if (max_solutions == sol_count)
        return true;

    // Apply DLX algorithm (recursive, non-deterministic)
    if (zeros()) { // success
        _solutions.emplace_back(_curSol);
        ++sol_count;
        return true;
    }

    auto curCol{ col_select() };
    if (0 == curCol->_size) // failure
        return false;

    // The recursive dance
    curCol->remove();
    for (auto cRow{ curCol->_head._d }; &curCol->_head != cRow; cRow = cRow->_d) {
        for (auto cCol{ cRow->_r }; cRow != cCol; cCol = cCol->_r) {
            cCol->_col->remove();
            _curSol.push_back(cCol->_row);
        }

        _solve(max_solutions, sol_count);

        for (auto cCol{ cRow->_l }; cRow != cCol; cCol = cCol->_l) {
            cCol->_col->restore();
            _curSol.pop_back();
        }
    }
    curCol->restore();
    return false;
}

/*****************************************************************************/
std::vector<DLX::Solution>
DLX::solve(uint32_t max_solutions) noexcept
{
    return pimpl->solve(max_solutions);
}

} // namespace ecv
