/**
 * @file nqueens.cpp
 * @brief Implementation of the N-Queens part of \a ecv.hpp
 * @author lhm
 */

// Project's headers
#include <ecv.hpp>

namespace ecv {

/*****************************************************************************/
State
NQueens::make_empty_state(size_t dim) noexcept
{
    return std::vector<std::string>(dim, std::string(dim, '0'));
}

/*****************************************************************************/
std::unique_ptr<NQueens>
NQueens::generate(const State& state) noexcept
{
    // Initial adjacency matrix dimensions ( without constraints )
    // - rows refer to the possible placements (placing a Queen in a cell : N * N )
    // - cols refer to the constraints
    //    - 1 queen per row                                     (N)
    //    - 1 queen per col                                     (N)
    //    - 1 queen per top-left  -> bot-right diagonal (D1)    (2 x (N - 1) - 1)
    //    - 1 queen per top-right -> bot-left  diagonal (D2)    (2 x (N - 1) - 1)
    int N(std::size(state)), rows{ N * N }, cols{ 6 * (N - 1) };

    // Primary constraints : The constraints that have to be satisfied exactly once
    // Initialy, there is 2 * N (N for rows and N for cols), but initial conditions
    // might modify (recude) it
    int primaryConstraints{ 2 * N };

    if (2 > N)
        return nullptr;

    for (const auto& line : state)
        if (N != static_cast<int>(std::size(line)))
            return nullptr;

    // Constraints ( non-zero nodes on provided inputs )
    auto authRows{ std::vector<int>(rows, 1) }, authCols{ std::vector<int>(cols, 1) };

    for (int i{ 0 }; i < N; ++i) {
        for (int j{ 0 }; j < N; ++j) {
            auto val{ state[i][j] - '0' };
            if (0 == val)
                continue; // No constraint on the node

            primaryConstraints -= 2;

            for (int k{ 0 }; k < N; ++k) { // cannot put a queen in the diagonals
                if ((i + k) < N && (j + k) < N)
                    authRows[(i + k) * N + (j + k)] = 0;
                if ((i + k) < N && (j - k) >= 0)
                    authRows[(i + k) * N + (j - k)] = 0;
                if ((i - k) >= 0 && (j + k) < N)
                    authRows[(i - k) * N + (j + k)] = 0;
                if ((i - k) >= 0 && (j - k) >= 0)
                    authRows[(i - k) * N + (j - k)] = 0;

                authRows[i * N + k] = 0; // cannot put queen in the line
                authRows[k * N + j] = 0; // cannot put queen in the col
            }

            authCols[i] = 0;     // row i constraint satisfied
            authCols[N + j] = 0; // col j constraint satisfied

            auto dst{ i + j }, diff{ abs(i - j) };
            // Remove already covered corner case
            if (auto idx{ 2 * N + N - 2 + i - j }; diff < (N - 1))
                authCols[idx] = 0; // D1 constraint satisfied
            // Remove already covered corner case
            if (auto idx{ 4 * (N - 1) + i + j }; ((0 != dst) && (2 * (N - 1) != dst)))
                authCols[idx] = 0; // D2 constraint satisfied
        }
    }

    auto R{ 0 }, C{ 0 };
    for (auto i{ 0 }; i < rows; ++i)
        authRows[i] = authRows[i] ? R++ : -1;
    for (auto i{ 0 }; i < cols; ++i)
        authCols[i] = authCols[i] ? C++ : -1;

    auto             adj{ std::vector<bool>(R * C, false) };
    std::vector<int> rowsList{};
    rowsList.reserve(R);

    for (auto i{ 0 }; i < N; ++i) {
        for (auto j{ 0 }; j < N; ++j) {
            auto r{ i * N + j };
            if (-1 == authRows[r])
                continue;
            rowsList.push_back(r);
            auto c1{ i }, c2{ N + j }, c3{ 2 * N + N - 2 + i - j }, c4{ 4 * (N - 1) + i + j };
            if (authCols[c1] != -1)
                adj[authRows[r] * C + authCols[c1]] = true;
            if (authCols[c2] != -1)
                adj[authRows[r] * C + authCols[c2]] = true;
            if (authCols[c3] != -1 && abs(i - j) < (N - 1))
                adj[authRows[r] * C + authCols[c3]] = true;
            if (authCols[c4] != -1 && (0 != (i + j) && 2 * (N - 1) != (i + j)))
                adj[authRows[r] * C + authCols[c4]] = true;
        }
    }

    struct shared_enabler : public NQueens
    {
        shared_enabler(const std::vector<bool>& data,
                       size_t                   rows,
                       size_t                   cols,
                       const std::vector<int>&  rowsList,
                       const State&             state,
                       int                      primary)
          : NQueens(data, rows, cols, rowsList, state, primary)
        {}
    };

    // In the N-Queens problem, only columns/rows constraints are primary.
    // Diagonal constraints are secondary, meaning it cannot be satisfied more than one time
    // but can be left unsatisfied.
    return std::make_unique<shared_enabler>(adj, R, C, rowsList, state, primaryConstraints);
}

/*****************************************************************************/
NQueens::NQueens(const std::vector<bool>& data,
                 size_t                   rows,
                 size_t                   cols,
                 const std::vector<int>&  rowsList,
                 const State&             initStata,
                 int                      primary) noexcept
  : ConcreteProblem(data, rows, cols, rowsList, primary)
  , _initState{ initStata }
{}

/*****************************************************************************/
State
NQueens::apply(const Solution& s) noexcept
{
    auto ret{ _initState };

    if (std::empty(ret))
        return ret;

    auto R{ std::size(_initState) }, C{ std::size(_initState[0]) };

    for (const auto& line : s._d)
        if (static_cast<size_t>(line) <= R * C)
            ret[line / R][line % R] = '1';

    return ret;
}

} // namespace ecv
