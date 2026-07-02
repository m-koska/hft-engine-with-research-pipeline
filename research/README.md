# Sample OFI Analysis


- [1. Theoretical Framework & PoC
  Scope](#1-theoretical-framework--poc-scope)
  - [Why Cont et al. (2014)?](#why-cont-et-al-2014)
  - [Scope of this PoC](#scope-of-this-poc)
- [2. Data Ingestion & Event-Time
  Preparation](#2-data-ingestion--event-time-preparation)
- [3. Multivariate Regression and Newey-West
  Adjustments](#3-multivariate-regression-and-newey-west-adjustments)
  - [Model Explanatory Power](#model-explanatory-power)
  - [Regression Coefficients](#regression-coefficients)
- [4. Empirical Price Impact Curves](#4-empirical-price-impact-curves)

## 1. Theoretical Framework & PoC Scope

### Why Cont et al. (2014)?

Before diving into the metrics, it helps to understand why we are using
the Order Flow Imbalance (OFI) framework. In their seminal 2014 paper,
*Price Dynamics in a Markovian Limit Order Book*, Rama Cont and his
co-authors proved something fundamental about markets: **the short-term
movement of stock prices is driven almost entirely by the supply/demand
imbalance of recent orders.** Instead of looking at raw price changes,
Cont’s framework aggregates all the buys, sells, and cancellations
happening at the top of the order book into a single metric: **OFI**.
The paper statistically proves that OFI has a powerful, linear
relationship with price changes.

### Scope of this PoC

For this Proof of Concept, we aren’t trying to build a complex,
production-ready trading algorithm. Our primary goal right now is order
book and replayer validation: **we are testing our data pipeline and our
NASDAQ replayer, proving my C++ engine useful.** To do that, we are
running a simplified version of Cont’s framework. Although it is very
easily retrievable, we skip market depth variable($D$). We are purely
checking using a simple regression model if OFI shows a predictable,
directional impact on mid-price returns within our pipeline.

If this simple regression yields the statistically significant results
that Cont’s theory predicts, it proves two things at once: 1. The NASDAQ
replayer and data pipeline are processing order book events accurately.
2. The underlying data is clean enough to support advanced predictive
modeling down the road.

To validate this setup, we estimate the following baseline model:

$$\Delta P_t = \beta_0 + \beta_1 OFI_t + \beta_2 OFI_{t-1} + \epsilon_t$$

Given the high-frequency nature of ITCH 5.0 event data, $\epsilon_t$ is
highly likely to exhibit serial correlation. To ensure statistical
validity, all $t$-statistics and $p$-values are adjusted using
**Newey-West (HAC) robust standard errors**.

## 2. Data Ingestion & Event-Time Preparation

``` r
library(pacman)
p_load(dplyr, ggplot2, purrr, tidyr, broom, knitr, sandwich, lmtest)

# Load data paths from the C++ replayer output
market_data <- read.csv("../cmake-build-release/ofi.csv")
stock_dict <- read.csv("../cmake-build-release/stock_dict.csv")

# Clean and transform data within the Event-Time paradigm
df_clean <- market_data |> 
  left_join(stock_dict, by = "stock_locate") |> 
  mutate(
    mid_price = mid_price / 10000,   # Convert to standard currency units
    stock_symbol = trimws(stock_symbol),
    mid_price_delta = mid_price_delta / 10000
  ) |>
  # Generate lag structure sequentially per instrument
  group_by(stock_symbol) |>
  arrange(timestamp) |> 
  mutate(
    ofi_lag1 = dplyr::lag(ofi, 1)
  ) |>
  tidyr::drop_na()
```

## 3. Multivariate Regression and Newey-West Adjustments

Standard OLS regressions tend to underestimate standard errors due to
time-series autocorrelation, which is highly prevalent in high-frequency
financial data. To account for this, we employ a robust Newey-West
covariance matrix estimator specified with a lag of 1. This adjustment
ensures that our $t$-statistics and $p$-values remain valid and are not
artificially inflated by the heteroskedasticity and serial correlation
inherent in the residuals.

``` r
# Fits linear regression with Newey-West Heteroskedasticity and Autocorrelation Consistent (HAC) errors
fit_order_flow_model <- function(stock_data_frame) {
  linear_model <- lm(mid_price_delta ~ ofi + ofi_lag1, data = stock_data_frame)
  
  # Estimate robust covariance matrix to account for high-frequency time-series autocorrelation
  hac_covariance_matrix <- sandwich::NeweyWest(linear_model, lag = 1, prewhite = FALSE)
  robust_coefficient_test <- lmtest::coeftest(linear_model, vcov = hac_covariance_matrix)
  
  list(
    r_squared = summary(linear_model)$r.squared,
    adjusted_r_squared = summary(linear_model)$adj.r.squared,
    coefficients = broom::tidy(robust_coefficient_test)
  )
}

# Run nested regression estimation per instrument across physical time buckets
models_nested <- df_clean |>
  group_by(stock_symbol) |>
  nest() |>
  mutate(
    model_fit = map(data, fit_order_flow_model),
    r_squared = map_dbl(model_fit, ~ .x$r_squared),
    adj_r_squared = map_dbl(model_fit, ~ .x$adjusted_r_squared),
    coefficients = map(model_fit, ~ .x$coefficients)
  )
```

### Model Explanatory Power

The $R^2$ and adjusted $R^2$ metrics capture the joint explanatory power
of the contemporaneous and lagged OFI variables on mid-price changes.

``` r
model_glance <- models_nested |>
  select(stock_symbol, r_squared, adj_r_squared)

kable(model_glance, digits = 4, caption = "Model Fit Statistics (10s Physical Time Buckets)")
```

| stock_symbol | r_squared | adj_r_squared |
|:-------------|----------:|--------------:|
| MSFT         |    0.3023 |        0.3020 |
| AAPL         |    0.3755 |        0.3752 |

Model Fit Statistics (10s Physical Time Buckets)

### Regression Coefficients

The estimated model coefficients ($\beta$) are presented below. All
standard errors, $t$-statistics, and corresponding $p$-values are
computed using Newey-West standard errors to ensure robustness against
high-frequency serial correlation and heteroskedasticity.

``` r
detailed_coefficients <- models_nested |>
  select(stock_symbol, coefficients) |>
  unnest(coefficients)

kable(detailed_coefficients |> select(stock_symbol, term, estimate, std.error, statistic, p.value), 
      digits = 6, caption = "Regression Coefficients (Newey-West HAC Adjusted)")
```

| stock_symbol | term        |  estimate | std.error | statistic |  p.value |
|:-------------|:------------|----------:|----------:|----------:|---------:|
| MSFT         | (Intercept) | -0.000784 |  0.000692 | -1.132022 | 0.257688 |
| MSFT         | ofi         |  0.000010 |  0.000001 | 10.209479 | 0.000000 |
| MSFT         | ofi_lag1    |  0.000001 |  0.000001 |  0.790322 | 0.429383 |
| AAPL         | (Intercept) | -0.000304 |  0.000967 | -0.314068 | 0.753485 |
| AAPL         | ofi         |  0.000021 |  0.000001 | 14.232514 | 0.000000 |
| AAPL         | ofi_lag1    | -0.000002 |  0.000001 | -4.152911 | 0.000033 |

Regression Coefficients (Newey-West HAC Adjusted)

## 4. Empirical Price Impact Curves

The scatter plots below trace the contemporaneous relationship between
current order flow imbalance ($OFI_t$) and immediate mid-price changes
($\Delta P_t$) across the selected instruments.

``` r
# Extract adjusted R-squared values for plot annotations
plot_labels <- models_nested |>
  mutate(label = paste0("Adj R² = ", round(adj_r_squared, 3))) |>
  select(stock_symbol, label)

ggplot(df_clean, aes(x = ofi, y = mid_price_delta)) +
  geom_point(aes(color = stock_symbol), alpha = 0.3, size = 1) +
  geom_smooth(method = "lm", formula = y ~ x, color = "black", fill = "gray80", se = TRUE, linewidth = 1) +
  facet_wrap(~ stock_symbol, scales = "free") + 
  geom_text(
    data = plot_labels,
    aes(label = label),
    x = -Inf, y = Inf, hjust = -0.1, vjust = 1.5,
    size = 4, fontface = "bold", color = "firebrick",
    inherit.aes = FALSE
  ) +
  theme_minimal(base_size = 12) +
  theme(
    strip.text = element_text(face = "bold", size = 13),
    legend.position = "none",
    panel.grid.minor = element_blank(),
    plot.title = element_text(face = "bold")
  ) +
  labs(
    title = "Contemporaneous Price Impact of OFI",
    subtitle = "Clock-Time aggregation: 10s physical time intervals",
    x = "Order Flow Imbalance (OFI)",
    y = "Mid-Price Delta"
  )
```

![](ofi_price_delta.png)
