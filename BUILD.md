# Build & Deployment Guide

## Local Development

### Prerequisites

| Tool | Purpose | Install |
|---|---|---|
| C compiler (`cc`/`gcc`) | Compile game | Included with most Linux distros, Xcode CLI tools on macOS |
| Raylib | Graphics library | See below |
| GNU Make | Build automation | Included with most systems |

### Install Raylib

**macOS:**
```sh
brew install raylib
```

**Ubuntu/Debian:**
```sh
# Raylib is not in default apt repos. Build from source:
sudo apt-get install -y build-essential git cmake libasound2-dev \
  libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev \
  libglu1-mesa-dev libxcursor-dev libxinerama-dev

git clone --depth 1 https://github.com/raysan5/raylib.git /tmp/raylib
cd /tmp/raylib/src && make PLATFORM=PLATFORM_DESKTOP
sudo make install
```

**Arch Linux:**
```sh
sudo pacman -S raylib
```

### Build & Run

```sh
make           # Compile dino_runner
make run       # Compile and run
make test      # Run unit tests
make clean     # Remove build artifacts
```

### WebGL Build (optional)

Requires [Emscripten](https://emscripten.org/docs/getting_started/downloads.html):

```sh
# Install Emscripten
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk && ./emsdk install latest && ./emsdk activate latest
source ./emsdk_env.sh

# Build
scripts/build_webgl.sh build/webgl

# Test locally
cd build/webgl && python3 -m http.server 8080
# Open http://localhost:8080 in browser
```

If Emscripten is not installed, the build script generates a placeholder HTML page.

---

## CI/CD Pipeline

**Workflow file:** `.github/workflows/release.yml`

### Pipeline structure

```
push/PR ──► test ──► build-webgl ──► deploy (release/* tags only)
```

| Job | Trigger | What it does |
|---|---|---|
| **test** | Every push & PR | Compiles and runs `make test` |
| **build-webgl** | After test passes | Builds WebGL package via Emscripten, uploads as artifact |
| **deploy** | Only on `release/*` tags | Downloads WebGL artifact, syncs to S3 |

### Triggering a release

```sh
git tag release/0.1.0
git push origin release/0.1.0
```

This runs all three jobs: test → build-webgl → deploy to S3.

---

## S3 Setup Guide

### Step 1: Create an S3 Bucket

```sh
aws s3 mb s3://your-dino-runner-bucket --region us-east-1
```

### Step 2: Enable Static Website Hosting

```sh
aws s3 website s3://your-dino-runner-bucket \
  --index-document index.html \
  --error-document index.html
```

### Step 3: Set Bucket Policy for Public Access

First, disable the "Block all public access" setting:

```sh
aws s3api put-public-access-block \
  --bucket your-dino-runner-bucket \
  --public-access-block-configuration \
    BlockPublicAcls=false,IgnorePublicAcls=false,BlockPublicPolicy=false,RestrictPublicBuckets=false
```

Then add a bucket policy for public read:

```sh
aws s3api put-bucket-policy \
  --bucket your-dino-runner-bucket \
  --policy '{
    "Version": "2012-10-17",
    "Statement": [
      {
        "Sid": "PublicReadGetObject",
        "Effect": "Allow",
        "Principal": "*",
        "Action": "s3:GetObject",
        "Resource": "arn:aws:s3:::your-dino-runner-bucket/*"
      }
    ]
  }'
```

Replace `your-dino-runner-bucket` with your actual bucket name.

### Step 4: Create an IAM User for CI/CD

```sh
# Create user
aws iam create-user --user-name dino-runner-ci

# Create access key
aws iam create-access-key --user-name dino-runner-ci
# Save the AccessKeyId and SecretAccessKey from the output
```

Attach a minimal policy:

```sh
aws iam put-user-policy \
  --user-name dino-runner-ci \
  --policy-name S3DeployPolicy \
  --policy-document '{
    "Version": "2012-10-17",
    "Statement": [
      {
        "Effect": "Allow",
        "Action": [
          "s3:PutObject",
          "s3:GetObject",
          "s3:ListBucket",
          "s3:DeleteObject"
        ],
        "Resource": [
          "arn:aws:s3:::your-dino-runner-bucket",
          "arn:aws:s3:::your-dino-runner-bucket/*"
        ]
      }
    ]
  }'
```

Replace `your-dino-runner-bucket` with your actual bucket name.

### Step 5: Configure GitHub Secrets

Go to your repo → **Settings** → **Secrets and variables** → **Actions** → **New repository secret**.

Add the following:

| Secret | Value |
|---|---|
| `S3_BUCKET` | `your-dino-runner-bucket` |
| `AWS_REGION` | `us-east-1` (or your region) |
| `AWS_ACCESS_KEY_ID` | From Step 4 output |
| `AWS_SECRET_ACCESS_KEY` | From Step 4 output |

### Step 6: Verify

Push a release tag:

```sh
git tag release/0.1.0
git push origin release/0.1.0
```

After the workflow completes, the game is accessible at:

```
http://your-dino-runner-bucket.s3-website-us-east-1.amazonaws.com
```

### Optional: CloudFront CDN

For HTTPS and better performance, add a CloudFront distribution in front of the S3 bucket:

```sh
aws cloudfront create-distribution \
  --origin-domain-name your-dino-runner-bucket.s3-website-us-east-1.amazonaws.com \
  --default-root-object index.html
```

This gives you an `https://d1234567890.cloudfront.net` URL with global edge caching.
