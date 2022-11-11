/**
 * @file sudoku.cpp
 * @brief Implementation of the sudoku part of \a ecv.hpp
 * @author lhm
 */

// Project's headers
#include <ecv.hpp>

namespace ecv {

/*****************************************************************************/
State
Sudoku::make_empty_state() noexcept
{
    return { "000000000", "000000000", "000000000", "000000000", "000000000",
             "000000000", "000000000", "000000000", "000000000" };
}

/*****************************************************************************/
std::unique_ptr<Sudoku>
Sudoku::generate(const State& state) noexcept
{
    static constexpr size_t N{ 9 };

    if (N != std::size(state))
        return nullptr;

    for (const auto& line : state)
        if (N != std::size(line))
            return nullptr;

    // Initial adjacency matrix dimensions ( without constraints )
    // - rows refer to the possible placements (placing a number in a cell : N * N * N)
    // - cols refer to the constraints
    //    - 1 number per cell (N * N)
    //    - each number once per row (N * N)
    //    - each number once per col (N * N)
    //    - each number once per 3*3 area (N * N)
    auto rows{ N * N * N }, cols{ 4 * N * N };

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
                authRows[i * N * N + j * N + k] = 0;       // cannot put any val in the cell
                authRows[i * N * N + k * N + val - 1] = 0; // cannot put val in the line
                authRows[k * N * N + j * N + val - 1] = 0; // cannot put val in the col
            }

            for (size_t k{ 0 }; k < 3; ++k) {
                for (size_t l{ 0 }; l < 3; ++l) {
                    // cannot put val in the 3*3 area
                    authRows[(k + 3 * (i / 3)) * N * N + (l + 3 * (j / 3)) * N + val - 1] = 0;
                }
            }

            authCols[i * N + j] = 0;                   // cell (i,j) constraint satisfied
            authCols[N * N + i * N + val - 1] = 0;     // row i constraint satisfied for val
            authCols[2 * N * N + j * N + val - 1] = 0; // col j constraint satisfied for val
            authCols[3 * N * N + (3 * (i / 3) + (j / 3)) * N + val - 1] =
              0; // area constraint satisfied for val
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
                size_t c1{ i * N + j }, c2{ N * N + i * N + k }, c3{ 2 * N * N + j * N + k },
                  c4{ 3 * N * N + (3 * (i / 3) + (j / 3)) * N + k };
                if (authCols[c1] != -1)
                    adj[authRows[r] * C + authCols[c1]] = true;
                if (authCols[c2] != -1)
                    adj[authRows[r] * C + authCols[c2]] = true;
                if (authCols[c3] != -1)
                    adj[authRows[r] * C + authCols[c3]] = true;
                if (authCols[c4] != -1)
                    adj[authRows[r] * C + authCols[c4]] = true;
            }
        }
    }

    struct shared_enabler : public Sudoku
    {
        shared_enabler(const std::vector<bool>& data,
                       size_t                   rows,
                       size_t                   cols,
                       const std::vector<int>&  rowsList,
                       const State&             state)
          : Sudoku(data, rows, cols, rowsList, state)
        {}
    };

    return std::make_unique<shared_enabler>(adj, R, C, rowsList, state);
}

/*****************************************************************************/
Sudoku::Sudoku(const std::vector<bool>& data,
               size_t                   rows,
               size_t                   cols,
               const std::vector<int>&  rowsList,
               const State&             initStata) noexcept
  : ConcreteProblem(data, rows, cols, rowsList)
  , _initState{ initStata }
{}

/*****************************************************************************/
State
Sudoku::apply(const Solution& s) noexcept
{
    auto ret{ _initState };

    if (std::empty(ret))
        return ret;

    auto R{ std::size(_initState) }, C{ std::size(_initState[0]) };

    for (const auto& line : s._d) {
        if (static_cast<size_t>(line) > R * R * C)
            return ret;
        auto pos{ line / R };
        auto val{ (line % R) + 1 };

        ret[pos / C][pos % C] = (0 == val) ? R + '0' : val + '0';
    }

    return ret;
}

} // namespace ecv
