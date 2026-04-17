import pandas as pd
import matplotlib.pyplot as plt

# plt.figure(figsize=(10, 6))

df = pd.read_csv('gc.csv')
plt.semilogy(df['iteration'], df['global_pso'], label='Global PSO')
plt.semilogy(df['iteration'], df['local_pso_ring'], label='Local PSO (Ring)')
plt.xlim(left=0)
plt.ylim(bottom=0)
plt.xlabel('Итерация')
plt.legend(loc='lower left')
plt.grid(True)
plt.ylabel('Лучшее значение фитнеса')
plt.title('Сравнение глобального и локального PSO')
plt.savefig('gc.png', dpi=150)
plt.show()
plt.close()

plt.figure(figsize=(10, 6))
df = pd.read_csv('in.csv')
df_early = df[df['iteration'] <= 5000]
df = df_early
labels = ['Постоянный ω = 0.7', 'Линейное убывание (0.9 → 0.4)', 'Нелинейное убывание (α = 0.99)']
columns = ['w_const_0.7', 'w_linear', 'w_nonlinear']
for col,  label in zip(columns,  labels):
    plt.semilogy(df['iteration'], df[col], label=label)
plt.xlim(0)
plt.ylim(0)
plt.xlabel('Итерация')
plt.ylabel('Лучшее значение фитнеса')
plt.title('Влияние веса инерции на сходимость PSO')
plt.legend(loc='lower left')
plt.grid(True)
plt.savefig('in.png', dpi=150)
plt.show()
plt.close()


df = pd.read_csv('sw.csv')
df_early = df[df['iteration'] <= 5000]
df = df_early
sizes = [10, 20, 30, 50, 100]
columns = ['n10', 'n20', 'n30', 'n50', 'n100']
for col,  size in zip(columns,  sizes):
    plt.semilogy(df['iteration'], df[col], label=f'n = {size}')
plt.xlim(0)
plt.ylim(0)
plt.xlabel('Итерация')
plt.ylabel('Лучшее значение фитнеса')
plt.title('Влияние размера роя инерции на сходимость PSO')
plt.legend(loc='lower left')
plt.grid(True)
plt.savefig('sw.png', dpi=150)
plt.show()
plt.close()


df = pd.read_csv('c.csv')
df_early = df[df['iteration'] <= 15000]
df = df_early
sizes = [10, 20, 30, 50, 100]
columns = ['n10', 'n20', 'n30', 'n50', 'n100']
configs = [
    ('c1=3_c2=1', 'c₁=3, c₂=1 (когнитивный)'),
    ('c1=2_c2=2', 'c₁=2, c₂=2 (баланс)'),
    ('c1=1_c2=3', 'c₁=1, c₂=3 (социальный)'),
    ('c1=0_c2=2', 'c₁=0, c₂=2 (только социальный)'),
    ('c1=2_c2=0', 'c₁=2, c₂=0 (только когнитивный)'),
]

for col,  label in configs:
    plt.semilogy(df['iteration'], df[col], label=label)
plt.xlim(0)
plt.ylim(0)
plt.xlabel('Итерация')
plt.ylabel('Лучшее значение фитнеса')
plt.title('Влияние коэффициентов c₁ и c₂ на сходимость PSO')
plt.legend(loc='lower left')
plt.grid(True)
plt.savefig('c.png', dpi=150)
plt.show()
plt.close()
