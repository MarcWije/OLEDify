from fastapi import APIRouter, Request
from fastapi.responses import RedirectResponse, JSONResponse
import os
import requests
import urllib.parse

router = APIRouter()

# Load from environment
CLIENT_ID = os.getenv("SPOTIFY_CLIENT_ID")
CLIENT_SECRET = os.getenv("SPOTIFY_CLIENT_SECRET")
REDIRECT_URI = os.getenv("SPOTIFY_REDIRECT_URI")
SCOPE = "user-library-read playlist-read-private"

# In-memory token storage (for practice)
TOKENS = {}

@router.get("/login")
def login():
    """Redirect user to Spotify login"""
    query_params = urllib.parse.urlencode({
        "client_id": CLIENT_ID,
        "response_type": "code",
        "redirect_uri": REDIRECT_URI,
        "scope": SCOPE,
    })
    url = f"https://accounts.spotify.com/authorize?{query_params}"
    return RedirectResponse(url)


@router.get("/callback")
def callback(request: Request):
    """Exchange code for access + refresh tokens"""
    code = request.query_params.get("code")
    if not code:
        return JSONResponse({"error": "No code provided"}, status_code=400)
    
    token_url = "https://accounts.spotify.com/api/token"
    payload = {
        "grant_type": "authorization_code",
        "code": code,
        "redirect_uri": REDIRECT_URI,
        "client_id": CLIENT_ID,
        "client_secret": CLIENT_SECRET,
    }
    
    response = requests.post(token_url, data=payload)
    if response.status_code != 200:
        return JSONResponse({"error": "Token exchange failed", "details": response.json()}, status_code=400)
    
    tokens = response.json()
    TOKENS["access_token"] = tokens["access_token"]
    TOKENS["refresh_token"] = tokens.get("refresh_token")
    
    return JSONResponse({"message": "Login successful! You can now call /playlists"})


@router.get("/playlists")
def get_playlists():
    """Fetch user playlists from Spotify API"""
    access_token = TOKENS.get("access_token")
    if not access_token:
        return JSONResponse({"error": "User not logged in"}, status_code=401)
    
    url = "https://api.spotify.com/v1/me/playlists"
    headers = {"Authorization": f"Bearer {access_token}"}
    
    response = requests.get(url, headers=headers)
    if response.status_code != 200:
        return JSONResponse({"error": "Failed to fetch playlists", "details": response.json()}, status_code=400)
    
    return response.json()
