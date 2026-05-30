library(pacman)
p_load(
  dplyr,
  ggplot2,
  purrr,
  tidyr
)

# 1. Loading data and cleanup

market_data <- read.csv("cmake-build-release/ofi.csv")
stock_dict <- read.csv("cmake-build-release/stock_dict.csv")

market_data <- market_data |> left_join(stock_dict, by = "stock_locate") |> 
  mutate(
    stock_symbol = trimws(stock_symbol),
    mid_price = mid_price / 10000,
    mid_price_delta = mid_price_delta / 10000,
  ) |>
  filter(mid_price_delta < 20) |>
  select(!c(timestamp, mid_price, stock_locate))

# 2. Linear model for every stock

market_data_lms <- market_data |>
  group_by(stock_symbol) |>
  nest() |>
  mutate(
    correlation = map_dbl(data, ~ cor(.x$ofi, .x$mid_price_delta, use = "complete.obs")),
    model = map(data, ~ lm (mid_price_delta ~ ofi, data = .x)),
    r_squared = map_dbl(model, ~ summary(.x)$r.squared),

    label = paste0("R² = ", round(r_squared, 3), "\nCorrelation = ", round(correlation, 3))
  ) |>
  select(stock_symbol, label)

# 3. Visualising data

graph <- ggplot(market_data, aes(x = ofi, y = mid_price_delta)) +
  
  geom_point(aes(color = stock_symbol), alpha = 0.3, size = 1.2) +
  
  geom_smooth(method = "lm", color = "black", fill = "gray80", se = TRUE, size = 1) +
  
  
  # separate facets
  facet_wrap(~ stock_symbol, scales = "free") + 
  
  geom_text(
    data = market_data_lms,
    aes(label = label),
    x = -Inf, y = Inf, hjust = -0.1, vjust = 1.2,
    size = 4, fontface = "bold", color = "gray20",
    inherit.aes = FALSE
  ) +
  theme_minimal(base_size = 13) +
  theme(
    strip.text = element_text(face = "bold", size = 14),
    legend.position = "none",
    panel.grid.minor = element_blank()
  ) +
  
  labs(
    title = "Sample linear regression model using parsed data",
    subtitle = "Impact of Order Flow Imbalance on price change\nAAPL and MSFT (30.01.2020)",
    x = "Order Flow Imbalance (OFI)",
    y = "Price delta"
  )

print(graph)
ggsave("ofi_price_delta.png")