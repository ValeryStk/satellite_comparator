from pystac_client import Client
import requests
from pathlib import Path
import json
from datetime import datetime

client = Client.open("https://earth-search.aws.element84.com/v1")
search = client.search(
    collections=["sentinel-2-l2a"],
    bbox=[14.3, 40.7, 14.6, 40.8],
    datetime="2025-08-20/2025-08-25",
    query={"eo:cloud_cover": {"lt": 20}}
)

output_dir = Path("sentinel_full")
metadata_dir = Path("sentinel_metadata")
output_dir.mkdir(exist_ok=True)
metadata_dir.mkdir(exist_ok=True)

total_scenes = 0
good_scenes = 0

def is_valid_http_url(url):
    """Проверяет HTTP URL"""
    return url.startswith(('http://', 'https://'))

for i, item in enumerate(search.items()):
    total_scenes += 1
    
    # ПРОВЕРКА ОБЛАЧНОСТИ <30%
    cloud_cover = item.properties.get('eo:cloud_cover', 100)
    print(f"\nСцена {i+1}/{total_scenes}: {item.id}")
    print(f"Облачность: {cloud_cover:.1f}%")
    
    if cloud_cover > 30.0:
        print("✗ ПРОПУЩЕНА (>30% облачности)")
        continue
    
    good_scenes += 1
    item_id = item.id.split('_')[-1].replace('.SAFE', '')
    print(f"✓ ПРИНЯТА ({cloud_cover:.1f}%)")
    
    # БЕЗОПАСНЫЕ каналы (работают через HTTP)
    safe_bands = {
        '10m': ['blue', 'green', 'red', 'nir'],
        '20m': ['rededge1', 'rededge2', 'nir08', 'nir09', 'swir16', 'swir22']
    }
    
    downloaded_bands = []
    for res_type, bands in safe_bands.items():
        for band_name in bands:
            if band_name in item.assets:
                url = item.assets[band_name].href
                
                # ПРОПУСК S3 URL
                if not is_valid_http_url(url):
                    print(f"  ✗ S3 URL пропущен: {band_name}")
                    continue
                
                save_path = output_dir / f"{item_id}_{band_name}_{res_type}.tif"
                
                try:
                    resp = requests.get(url, timeout=30)
                    resp.raise_for_status()
                    with open(save_path, 'wb') as f:
                        f.write(resp.content)
                    print(f"  ✓ {band_name} ({res_type})")
                    downloaded_bands.append(f"{band_name}_{res_type}")
                except Exception as e:
                    print(f"  ✗ Ошибка {band_name}: {e}")
    
    # JSON метаданные
    metadata = {
        "id": item.id,
        "datetime": item.datetime.isoformat() if item.datetime else None,
        "eo_cloud_cover": float(cloud_cover),
        "sun_azimuth": item.properties.get('view:sun_azimuth'),
        "sun_zenith": item.properties.get('view:sun_zenith'),
        "bands_downloaded": downloaded_bands,
        "radiometric": {"add_offset": -0.1, "scale": 0.0001},
        "download_timestamp": datetime.now().isoformat()
    }
    
    meta_path = metadata_dir / f"{item_id}_metadata.json"
    with open(meta_path, 'w', encoding='utf-8') as f:
        json.dump(metadata, f, ensure_ascii=False, indent=2)
    print(f"  ✓ Метаданные: {meta_path.name}")

print(f"\n=== РЕЗУЛЬТАТ ===")
print(f"Всего сцен: {total_scenes}")
print(f"Чистых (<30% облачности): {good_scenes}")
print(f"Данные: {output_dir}")
print(f"Метаданные: {metadata_dir}")
