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
    DLX(const std::vector<bool>& data, size_t rows, size_t cols, const std::vector<int>& rowsList)
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
    /*!
     * \brief generate Generate a data structure corresponding to the "latin squares"
     * exact cover problem.
     * \param state a String representation of the problem as a grid.
     * Use '0' to represent non-constrained cells
     * \return An exact cover problem pointer in case of success, nullptr otherwise
     */
    static std::unique_ptr<LatinSquares> generate(const State& state) noexcept;

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

} // namespace ecv

#endif // INCLUDE_ECV_HPP
