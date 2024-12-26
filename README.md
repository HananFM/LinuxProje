# Linux Shell Uygulaması

Bu proje, İşletim Sistemleri dersi için geliştirilmiş basit bir Linux shell uygulamasıdır.

## Grup Üyeleri
#[Hanan Haidar-B211210581]
#[Reem Almadhagi-Y245012049]
#[Dania Almassri-B221210569]
#[tasnim abdulhay-B211210585]

## Özellikler
1. **Komut Prompt (5 puan)**
   - Her komuttan sonra ">" promptu gösterilir
   - Prompt buffer'ı düzgün şekilde temizlenir

2. **Built-in Komutlar (5 puan)**
   - `quit`: Shell'den çıkış yapar
   - Arkaplan prosesleri varsa, bunların tamamlanmasını bekler

3. **Tekli Komut İcrası (10 puan)**
   - Tek komut ve argümanlarını çalıştırma
   - Hata durumlarını yönetme

4. **Giriş Yönlendirme (20 puan)**
   - `<` operatörü ile dosyadan girdi alma
   - Dosya bulunamama durumunu kontrol etme

5. **Çıkış Yönlendirme (20 puan)**
   - `>` operatörü ile çıktıyı dosyaya yazma
   - Dosya oluşturma ve yazma izinlerini kontrol etme

6. **Arkaplan Çalışma (20 puan)**
   - `&` operatörü ile komutları arkaplanda çalıştırma
   - Proses durumlarını takip etme ve raporlama
   - SIGCHLD sinyali ile sonlanan prosesleri temizleme

7. **Boru (Pipe) İşlemleri (20 puan)**
   - `|` operatörü ile komutlar arası veri aktarımı
   - Çoklu proses yönetimi

## Derleme
```bash
make
```

## Çalıştırma
```bash
./shell
```

## Kullanım Örnekleri

1. **Temel Komutlar**
```bash
> ls -l
> pwd
> echo "Hello World"
```

2. **I/O Yönlendirme**
```bash
> cat < input.txt
> echo "Test" > output.txt
> cat input.txt > output.txt
```

3. **Boru (Pipe) İşlemleri**
```bash
> ls | grep .txt
> cat file.txt | wc -l
```

4. **Arkaplan İşlemleri**
```bash
> sleep 10 &
> find / -name "*.txt" &
```

5. **Çoklu Komutlar**
```bash
> echo "First"; echo "Second"; echo "Third"
> ls -l & echo "Background"; pwd
```

## Hata Yönetimi
- Dosya bulunamama durumları
- Komut çalıştırma hataları
- Bellek yönetimi
- Sinyal işleme

## Katkıda Bulunma
1. Bu depoyu fork edin
2. Yeni bir özellik dalı oluşturun (`git checkout -b feature/amazing-feature`)
3. Değişikliklerinizi commit edin (`git commit -m 'Add some amazing feature'`)
4. Dalınıza push yapın (`git push origin feature/amazing-feature`)
5. Bir Pull Request oluşturun
