/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

export class User {
  public token: String;
  public expires_in: String; //过期时间
  public refresh_token: String; //刷新token
  public expires_at: String; //时间
  public token_type: String; //token类型

  constructor(token?: String, expires_in?: String, refresh_token?: String, expires_at?: String, token_type?: String) {
    this.token = token;
    this.expires_in = expires_in;
    this.refresh_token = refresh_token;
    this.expires_at = expires_at;
    this.token_type = token_type;
  }
}
