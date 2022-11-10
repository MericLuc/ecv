/**
 * @file ecv.hpp
 * @brief Main header of the ecv library
 * @author lhm
 */

#ifndef INCLUDE_ECV_HPP
#define INCLUDE_ECV_HPP

// Standard headers
#include <limits>
#include <memory>
#include <string>
#include <vector>

/*!
 * \brief Main namespace of the \a ecv library
 */
namespace ecv {

/*!
 * \brief State is the representation of an exact cover problem state
 */
typedef std::vector<std::string> State;

/*!
 * \brief The LatinSquares class is the DLX implementation of an exact cover problem
 * \see https://arxiv.org/pdf/cs/0011047v1.pdf for more informations about
 * DLX (dancing links) algorithm.
 *
 * It basically is just a way of implementing backtracking, recursive, DFS algorithm
 * to solve exact cover problems using circular double linked lists.
 */
class DLX
{
protected:
    struct Solution
    { ///< A solution is a combinaison of rows
        explicit Solution(const std::vector<int>& data) noexcept
          : _d{ data }
        {}

        const std::vector<int> _d;
    };

public:
    virtual std::vector<Solution> solve(
      uint32_t max_solutions = std::numeric_limits<uint32_t>::max()) noexcept;
    virtual State apply(const Solution& s) noexcept = 0;

protected:
    /*!
     * \brief DLX Create a DLX algorithm
     * \param data The data of the adjacency matrix
     * \param rows The number of rows in \a data
     * \param cols The number of cols in \a data
     * \param rowsList The list of row identifiers (usefull to parse problem state from solutions)
     * \param primary The number of primary constraints.
     * By default, every constraint is primary (meaning it has to be satisfied exactly once)
     * Columns past \a primary index will be considered as 'secondary' (meaning the can be left
     * unsatisfied)
     */
    DLX(const std::vector<bool>& data,
        size_t                   rows,
        size_t                   cols,
        const std::vector<int>&  rowsList,
        int                      primary = -1)
    noexcept;
    virtual ~DLX() noexcept = default;

protected:
    struct Impl;
    std::shared_ptr<Impl> pimpl{ nullptr };
};

/*!
 * \brief The LatinSquares class is the implementation of the "latin squares" exact cover problem
 * \see https://en.wikipedia.org/wiki/Latin_square for more informations about "latin squares"
 */
class LatinSquares : public DLX
{
public:
    static State make_empty_state(size_t rows = 1, size_t cols = 1) noexcept;

public:
    /*!
     * \brief generate Generate a data structure corresponding to the "latin squares"
     * exact cover problem.
     * \param state a String representation of the problem as a grid.
     * Use '0' to represent non-constrained cells
     * \return An exact cover problem pointer in case of success, nullptr otherwise
     */
    static std::unique_ptr<LatinSquares> generate(const State& state = make_empty_state()) noexcept;

    State apply(const Solution& s) noexcept override;

    virtual ~LatinSquares() noexcept = default;

protected:
    LatinSquares(const std::vector<bool>& data,
                 size_t                   rows,
                 size_t                   cols,
                 const std::vector<int>&  rowsList,
                 const State&             initStata) noexcept;

private:
    const State _initState;
};

/*!
 * \brief The Sudoku class is the implementation of the "sudoku" exact cover problem
 * \see https://en.wikipedia.org/wiki/sudoku for more informations about "sudoku"
 */
class Sudoku : public DLX
{
public:
    static State make_empty_state() noexcept;

public:
    /*!
     * \brief generate Generate a data structure corresponding to the "sudoku"
     * exact cover problem.
     * \param state a String representation of the problem as a grid.
     * Use '0' to represent non-constrained cells
     * \return An exact cover problem pointer in case of success, nullptr otherwise
     */
    static std::unique_ptr<Sudoku> generate(const State& state = make_empty_state()) noexcept;

    State apply(const Solution& s) noexcept override;

    virtual ~Sudoku() noexcept = default;

protected:
    Sudoku(const std::vector<bool>& data,
           size_t                   rows,
           size_t                   cols,
           const std::vector<int>&  rowsList,
           const State&             initStata) noexcept;

private:
    const State _initState;
};

/*!
 * \brief The NQueens class is the implementation of the "N Queen" exact cover problem, which is a
 * generalization of the initial 8-Queen problem (\see
 * https://en.wikipedia.org/wiki/Eight_queens_puzzle) for more informations about.
 */
class NQueens : public DLX
{
public:
    static State make_empty_state(size_t dim = 8) noexcept;

public:
    /*!
     * \brief generate Generate a data structure corresponding to the "N-Queen" exact cover problem.
     * \param state a String representation of the problem as a grid.
     * Use '0' to represent non-constrained (i.e. empty) cells, everything else for a cell with a
     * Queen. \return An exact cover problem pointer in case of success, nullptr otherwise
     */
    static std::unique_ptr<NQueens> generate(const State& state = make_empty_state()) noexcept;

    State apply(const Solution& s) noexcept override;

    virtual ~NQueens() noexcept = default;

protected:
    NQueens(const std::vector<bool>& data,
            size_t                   rows,
            size_t                   cols,
            const std::vector<int>&  rowsList,
            const State&             initStata,
            int                      primary) noexcept;

private:
    const State _initState;
};

} // namespace ecv

#endif // INCLUDE_ECV_HPP
