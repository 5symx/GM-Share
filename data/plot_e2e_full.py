import matplotlib.pyplot as plt
import numpy as np

# Model and group setup
models = ['1B', '4B', '27B']
groups = ['naive', 'MPS', 'GM-Share']
group_colors = ['skyblue', 'lightgreen', 'lightcoral']

# Timing data
init_times = {
    '1B': [0.771, 0.75, 0.634],
    '4B': [1.197, 1.175, 0.997],
    '27B': [3.999, 3.986, 3.674]
}
decode_times = {
    '1B': [0.296, 0.278, 0.338],
    '4B': [0.422, 0.404, 0.417],
    '27B': [1.365, 1.533, 0.969]
}

# e2e_times = {
#     '1B': [1.067, 1.028, 0.972],
#     '4B': [1.619, 1.579, 1.414],
#     '27B': [5.364, 5.519, 4.643]
# }

x = np.arange(len(models))
width = 0.2
offsets = [-width, 0, width]

# plt.style.use('seaborn-v0_8')
fig, ax = plt.subplots(figsize=(10, 6))

# Plot bars
for i, group in enumerate(groups):
    init = [init_times[model][i] for model in models]
    decode = [decode_times[model][i] for model in models]
    pos = x + offsets[i]
    ax.bar(pos, init, width, label=f'{group} Load', color=group_colors[i])
    ax.bar(pos, decode, width, bottom=init, label=f'{group} Decode', color=group_colors[i], alpha=0.6, hatch='/')

# Labels
ax.set_xlabel('Model Size')
ax.set_ylabel('E2E Time (seconds)')
# ax.set_title('Init and Decode Time Breakdown by Model and Group')
ax.set_xticks(x)
ax.set_xticklabels(models)

# Legend above the plot in a single row
# ax.legend(loc='lower center', bbox_to_anchor=(0.5, 1.15), ncol=3, fontsize='medium')
ax.legend(loc='lower center', bbox_to_anchor=(0.5, 1.02), ncol=6, fontsize='medium')

plt.tight_layout()
plt.savefig('grouped_model_time_breakdown_top_legend.png', bbox_inches='tight')
plt.close()
