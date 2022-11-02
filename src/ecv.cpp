/**
 * @file ecv.cpp
 * @brief Implementation of \a ecv.hpp
 * @author lhm
 */

// Standard headers

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
    std::stack<int>       _curSol;

    Column*               col_select(void) noexcept;
    [[maybe_unused]] bool init(const std::vector<bool>& data,
                               size_t                   R,
                               size_t                   C,
                               const std::vector<int>&  rowsList) noexcept;
    std::vector<Solution> solve(bool) noexcept;
    bool                  _solve(bool) noexcept;

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
DLX::Impl::solve(bool all) noexcept
{
    _solutions.clear();
    if (!std::empty(_nodes))
        _solve(all);
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
DLX::Impl::_solve(bool all) noexcept
{
    if (!all && !std::empty(_solutions))
        return true;

    // Apply DLX algorithm (recursive, non-deterministic)
    if (zeros()) { // success
        _solutions.emplace_back(_curSol);
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
            _curSol.push(cCol->_row);
        }

        _solve(all);

        for (auto cCol{ cRow->_l }; cRow != cCol; cCol = cCol->_l) {
            cCol->_col->restore();
            _curSol.pop();
        }
    }
    curCol->restore();
    return false;
}

/*****************************************************************************/
std::vector<DLX::Solution>
DLX::solve(bool all) noexcept
{
    return pimpl->solve(all);
}

/*****************************************************************************/
std::unique_ptr<LatinSquares>
LatinSquares::generate(const State& state) noexcept
{
    // Initial adjacency matrix dimensions ( without constraints )
    auto N{ std::size(state) }, rows{ N * N * N }, cols{ 3 * N * N };

    for (const auto& line : state)
        if (N != std::size(line))
            return nullptr;

    // Constraints ( non-zero nodes on provided inputs )
    auto authRows{ std::vector<int>(rows, 1) }, authCols{ std::vector<int>(cols, 1) };

    for (size_t i{ 0 }; i < N; ++i) {
        for (size_t j{ 0 }; j < N; ++j) {
            auto val{ state[i][j] - '0' };
            if (0 == val)
                continue; // No constraint on the node

            for (size_t k{ 0 }; k < N; ++k) {
                authRows[i * N * N + j * N + k] = 0;
                authRows[i * N * N + k * N + val - 1] = 0;
                authRows[k * N * N + j * N + val - 1] = 0;
            }

            authCols[i * N + j] = 0;
            authCols[N * N + i * N + val - 1] = 0;
            authCols[2 * N * N + j * N + val - 1] = 0;
        }
    }

    size_t R{ 0 }, C{ 0 };
    for (size_t i{ 0 }; i < rows; ++i)
        authRows[i] = authRows[i] ? R++ : -1;
    for (size_t i{ 0 }; i < cols; ++i)
        authCols[i] = authCols[i] ? C++ : -1;

    auto             adj{ std::vector<bool>(R * C, false) };
    std::vector<int> rowsList{};
    rowsList.reserve(R);

    for (size_t i{ 0 }; i < N; ++i) {
        for (size_t j{ 0 }; j < N; ++j) {
            for (size_t k{ 0 }; k < N; ++k) {
                size_t r{ i * N * N + j * N + k };
                if (-1 == authRows[r])
                    continue;
                rowsList.push_back(r);
                size_t c1{ i * N + j }, c2{ N * N + i * N + k }, c3{ 2 * N * N + j * N + k };
                if (authCols[c1] != -1)
                    adj[authRows[r] * C + authCols[c1]] = true;
                if (authCols[c2] != -1)
                    adj[authRows[r] * C + authCols[c2]] = true;
                if (authCols[c3] != -1)
                    adj[authRows[r] * C + authCols[c3]] = true;
            }
        }
    }

    struct shared_enabler : public LatinSquares
    {
        shared_enabler(const std::vector<bool>& data,
                       size_t                   rows,
                       size_t                   cols,
                       const std::vector<int>&  rowsList,
                       const State&             state)
          : LatinSquares(data, rows, cols, rowsList, state)
        {}
    };

    return std::make_unique<shared_enabler>(adj, R, C, rowsList, state);
}

/*****************************************************************************/
LatinSquares::LatinSquares(const std::vector<bool>& data,
                           size_t                   rows,
                           size_t                   cols,
                           const std::vector<int>&  rowsList,
                           const State&             initStata) noexcept
  : DLX(data, rows, cols, rowsList)
  , _initState{ initStata }
{}

/*****************************************************************************/
State
LatinSquares::apply(const Solution& s) noexcept
{
    // TODO
    // - optimize
    // - handle error cases

    auto ret{ _initState };

    if (std::empty(ret))
        return ret;

    const std::vector<int> contents{ &s._d.top() + 1 - std::size(s._d), &s._d.top() + 1 };
    auto                   R{ std::size(_initState) }, C{ std::size(_initState[0]) };

    for (const auto& line : contents) {
        if (static_cast<size_t>(line) > R * R * C)
            return ret;
        auto pos{ line / R };
        auto val{ line % R };

        ret[pos / C][pos % C] = (0 == val) ? R + '0' : val + '0';
    }

    return ret;
}

} // namespace ecv
