
import matplotlib.pyplot as plt
import numpy as np

# Raw data
models = ['1B', '4B', '27B']
naive = np.array([0.774, 1.199, 4.022])
tmm = np.array([0.634, 0.997, 3.674])

# Normalization
naive_norm = naive / naive
tmm_norm = tmm / naive

# Speed-up (Naïve / TMM)
speedup = (naive - tmm) / naive * 100

x = np.arange(len(models))
width = 0.25

# Main plot
fig, ax = plt.subplots(figsize=(8, 5))


bars_naive = ax.bar(x - width/2, naive_norm, width, label='Naïve', color='#1f77b4')
bars_tmm = ax.bar(x + width/2, tmm_norm, width, label='TMM', color='#ff7f0e')
# ax.bar_label(bars_tmm, fmt='%.2f', padding=3, fontsize=10)

# Primary axis labels
ax.set_xlabel('Model Size', fontsize=12)
ax.set_ylabel('Normalized Latency for Workload Switching', fontsize=12)
# ax.set_title('Switching Latency Comparison with Speed-Up Overlay', fontsize=14)
ax.set_xticks(x)
ax.set_xticklabels(models)
ax.grid(axis='y', linestyle='--', alpha=0.6)

# Secondary axis for speed-up
ax2 = ax.twinx()
ax2.plot(x, speedup, marker='s', color='#333333', linestyle='--', linewidth=2)
ax2.set_ylabel('Speed-Up Percentage(%)', fontsize=12)
ax2.set_ylim(0, 50)  # adjust as needed for clarity

# Optional: Add speed-up values as text above points
for i in range(len(x)):
    ax2.text(x[i], speedup[i]+3, f'{speedup[i]:.2f}%', ha='center', va='bottom', fontsize=10, color='#333333')

# Legends
# ax.legend(loc='upper left', fontsize=10)

# Layout for publication
plt.tight_layout()
plt.savefig('normalized_latency_speedup_overlay.png', dpi=300)
plt.show()