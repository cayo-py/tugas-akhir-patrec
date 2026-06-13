import pandas as pd
import numpy as np

# Konfigurasi jumlah data
NUM_ROWS = 200

# Inisialisasi seed agar data konsisten jika diulang
np.random.seed(42)

# 1. Generate Fitur
jumlah_halaman = np.random.randint(100, 500, size=NUM_ROWS)
jenis_cover = np.random.choice(['Softcover', 'Hardcover'], size=NUM_ROWS, p=[0.7, 0.3])
kualitas_cetak = np.random.choice(['Bookpaper', 'HVS', 'Art Paper'], size=NUM_ROWS, p=[0.5, 0.3, 0.2])

# Daftar 5 penerbit
daftar_penerbit = [
    'Gramedia Pustaka Utama', 'Mizan', 'Erlangga',
    'Buku Mojok', 'Marjin Kiri' 
]
# Probabilitas: GPU (30%), Mizan (20%), Erlangga (20%), Buku Mojok (15%), Marjin Kiri (15%)
penerbit = np.random.choice(daftar_penerbit, size=NUM_ROWS, p=[0.3, 0.2, 0.2, 0.15, 0.15])

# 2. Logic Pembentukan Harga (Pattern Recognition Target)
harga = np.zeros(NUM_ROWS)

for i in range(NUM_ROWS):
    # Base price (Harga dasar)
    base = 15000
    
    # Pengaruh halaman (misal: Rp 150 per halaman)
    cost_halaman = jumlah_halaman[i] * 150
    
    # Pengaruh jenis cover
    cost_cover = 35000 if jenis_cover[i] == 'Hardcover' else 15000
    
    # Pengaruh kualitas cetak
    if kualitas_cetak[i] == 'Art Paper':
        cost_cetak = 25000
    elif kualitas_cetak[i] == 'HVS':
        cost_cetak = 10000
    else: # Bookpaper
        cost_cetak = 5000
        
    # Pengaruh penerbit (Mayor vs Indie)
    if penerbit[i] in ['Gramedia Pustaka Utama', 'Mizan', 'Erlangga']:
        cost_penerbit = 20000  # Brand value / biaya distribusi nasional
    else:
        cost_penerbit = 10000   # Penerbit indie biasanya memiliki overhead lebih kecil
    
    # Random noise untuk mensimulasikan fluktuasi dunia nyata (+/- Rp 2000 - 8000)
    noise = np.random.normal(0, 5000)
    
    # Total kalkulasi
    total_harga = base + cost_halaman + cost_cover + cost_cetak + cost_penerbit + noise
    
    # Pembulatan ke 500 terdekat agar terlihat seperti harga riil
    harga[i] = round(total_harga / 500) * 500

# 3. Pembuatan DataFrame dan Export ke CSV
df = pd.DataFrame({
    'jumlah_halaman': jumlah_halaman,
    'jenis_cover': jenis_cover,
    'kualitas_cetak': kualitas_cetak,
    'penerbit': penerbit,
    'harga': harga.astype(int)
})

# Cek beberapa sampel data
print(df.head(10))

# Simpan ke CSV
df.to_csv('Dataset/dataset_harga_buku.csv', index=False)
print(f"\nDataset berhasil dibuat dengan {NUM_ROWS} baris data dan 5 penerbit Indonesia.")